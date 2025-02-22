#include "CryptoHelper.h"

using namespace std;

SecureString CryptoHelper::pbkdf2_derive_key(const SecureString& password,
											 const vector<uint8_t>& salt,
											 int iterations)
{
	// EVP_MAX_KEY_LENGTH gibt die maximale Schl�ssell�nge vor.
	vector<uint8_t> key(EVP_MAX_KEY_LENGTH);

    // F�hre die Schl�sselableitung mit dem PBKDF2-Algorithmus durch.
    // - password.c_str(): Zeiger auf die Passwort-Zeichenkette.
    // - password.size(): L�nge des Passworts.
    // - salt.data() und salt.size(): Zeiger und L�nge des Salts.
    // - iterations: Anzahl der Iterationen, um Brute-Force-Angriffe zu erschweren.
    // - EVP_sha256(): Verwendet den SHA-256 Hash-Algorithmus.
    // - key.size() und key.data(): L�nge und Speicherbereich des abgeleiteten Schl�ssels.

    if (!PKCS5_PBKDF2_HMAC(password.c_str(), password.size(), salt.data(),
        salt.size(), iterations, EVP_sha256(), key.size(), key.data()))
    {
        throw runtime_error("Key Derivation fehlgeschlagen");
    }

    return SecureString(reinterpret_cast<const char*>(key.data()), key.size());
}

// Implementierung der AES-Verschl�sselung in AES-256-CBC
vector<uint8_t> CryptoHelper::aes_encrypt(const SecureString& plaintext, const SecureString& key)
{
    // Erzeuge einen neuen Kontext f�r die Verschl�sselung
    EVP_CIPHER_CTX* ctx = EVP_CIPHER_CTX_new();

    if (!ctx)
    {
        throw runtime_error("Konnte EVP_CIPHER_CTX nicht erstellen");
    }

    // Bestimme die L�nge des Initialisierungsvektors (IV) f�r AES-256-CBC
    int iv_length = EVP_CIPHER_iv_length(EVP_aes_256_cbc());
    vector<uint8_t> iv(iv_length);

    // Generiere einen kryptographischen zuf�lligen iv
    if (RAND_bytes(iv.data(), iv_length) != 1)
    {
        EVP_CIPHER_CTX_free(ctx);
        throw runtime_error("IV-Generate Error");
    }

    // Initialisiere den Verschl�sselungskontext mit AES-256-CBC, dem Schl�ssel und dem IV
    if (EVP_EncryptInit_ex(ctx, EVP_aes_256_cbc(), nullptr,
        reinterpret_cast<const unsigned char*>(key.c_str()), iv.data()) != 1)
    {
        EVP_CIPHER_CTX_free(ctx);
        throw runtime_error("Verschl�sselungsinitialisierung fehlgeschlagen");
    }

    // Ermittle die Blockgr��e des Verschl�sselungsalgorithmus
    int block_size = EVP_CIPHER_block_size(EVP_aes_256_cbc());

    // Berechne die maximale L�nge des Ciphertexts (Klartextgr��e + ein Block f�r Padding)
    int ciphertext_len = plaintext.size() + block_size;
    vector<uint8_t> ciphertext(ciphertext_len);

    int len;
    // Verschl�ssle den Klartext in Bl�cken
    if (EVP_EncryptUpdate(ctx, ciphertext.data(), &len,
        reinterpret_cast<const unsigned char*>(plaintext.c_str()), plaintext.size()) != 1)
    {
        EVP_CIPHER_CTX_free(ctx);
        throw runtime_error("Verschl�sselungsupdate fehlgeschlagen");
    }

    int total_len = len;

    // Finalisiere die Verschl�sselung, um das letzte Datenst�ck (inc. Padding zu verarbeiten
    if (EVP_EncryptFinal_ex(ctx, ciphertext.data() + len, &len) != 1)
    {
        EVP_CIPHER_CTX_free(ctx);
        throw runtime_error("Verschl�sselungs-finalisierung fehlgeschlagen");
    }

    total_len += len;

    // Passe die Gr��e des Ciphertext-Vektors an die tats�chliche L�nge der verschl�sselten Daten an.
    ciphertext.resize(total_len);

    // F�ge den IV am Anfang des Ciphertexts ein, damit er beim verschl�sseln verf�gbar ist.
    ciphertext.insert(ciphertext.begin(), iv.begin(), iv.end());

    // Gib den Verschl�sselungskontext frei.
    EVP_CIPHER_CTX_free(ctx);
    return ciphertext;
}

// Implementierung der AES - Entschl�sselung im Modus AES - 256 - CBC
SecureString CryptoHelper::aes_decrypt(const std::vector<uint8_t>&ciphertext,
    const SecureString & key) 
{
    // Erzeuge einen neuen Kontext f�r die Entschl�sselung.
    EVP_CIPHER_CTX* ctx = EVP_CIPHER_CTX_new();
    if (!ctx)
        throw std::runtime_error("Konnte EVP_CIPHER_CTX nicht erstellen");

    // Bestimme die L�nge des IVs.
    int iv_length = EVP_CIPHER_iv_length(EVP_aes_256_cbc());
    // �berpr�fe, ob der Ciphertext lang genug ist, um einen IV zu enthalten.
    if (ciphertext.size() < static_cast<size_t>(iv_length))
    {
        EVP_CIPHER_CTX_free(ctx);
        throw std::runtime_error("Ciphertext zu kurz");
    }

    // Extrahiere den IV aus dem Anfang des Ciphertexts.
    std::vector<uint8_t> iv(ciphertext.begin(), ciphertext.begin() + iv_length);
    // Der eigentliche verschl�sselte Text befindet sich nach dem IV.
    std::vector<uint8_t> actual_ciphertext(ciphertext.begin() + iv_length, ciphertext.end());

    // Initialisiere den Entschl�sselungskontext mit AES-256-CBC, dem Schl�ssel und dem extrahierten IV.
    if (EVP_DecryptInit_ex(ctx, EVP_aes_256_cbc(), nullptr,
        reinterpret_cast<const unsigned char*>(key.c_str()), iv.data()) != 1) 
    {
        EVP_CIPHER_CTX_free(ctx);
        throw std::runtime_error("Entschl�sselungsinitialisierung fehlgeschlagen");
    }

    // Bereite einen Puffer vor, um den entschl�sselten Klartext aufzunehmen.
    int plaintext_len = actual_ciphertext.size();
    int block_size = EVP_CIPHER_block_size(EVP_aes_256_cbc());
    std::vector<uint8_t> plaintext(plaintext_len + block_size);

    int len;
    // Entschl�ssele den Ciphertext in Bl�cken.
    if (EVP_DecryptUpdate(ctx, plaintext.data(), &len,
        actual_ciphertext.data(), actual_ciphertext.size()) != 1) 
    {
        EVP_CIPHER_CTX_free(ctx);
        throw std::runtime_error("Entschl�sselungsupdate fehlgeschlagen");
    }
    int total_len = len;

    // Finalisiere die Entschl�sselung, um das Padding zu verarbeiten.
    if (EVP_DecryptFinal_ex(ctx, plaintext.data() + len, &len) != 1) 
    {
        EVP_CIPHER_CTX_free(ctx);
        throw std::runtime_error("Entschl�sselungsfinalisierung fehlgeschlagen");
    }
    total_len += len;
    // Passe die Gr��e des Klartext-Puffers an die tats�chliche L�nge der entschl�sselten Daten an.
    plaintext.resize(total_len);

    // Gib den Entschl�sselungskontext frei.
    EVP_CIPHER_CTX_free(ctx);

    // Gib den entschl�sselten Klartext als SecureString zur�ck.
    return SecureString(reinterpret_cast<const char*>(plaintext.data()), plaintext.size());
}

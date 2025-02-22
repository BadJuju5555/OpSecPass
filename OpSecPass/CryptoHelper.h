#pragma once

#include "SecureString.h"
#include <vector>
#include <stdexcept>
#include <openssl/evp.h>
#include <openssl/rand.h>
#include <openssl/sha.h>

using namespace std;

class CryptoHelper
{
public:
	//=========================================================================//
	//Leitet mit PBKDF2 einen Schl�ssel aus einen Passwort und einen Salt ab   //
	// - password: Dass Passwort als SecureString.							   //
	// - salt: Ein Zufallswert, der als vector von Bytes �bergeben wird		   //
	// - iterations: Anzahl der Iteration (Schutzma�nahme gegen Brute-Force)   //
	//=========================================================================//
	static SecureString pbkdf2_derive_key(const SecureString& password,
										const vector<uint8_t>& salt,
										int iterations = 10000);

	//==========================================================================//
	// Verschl�sselt einen Klartext mittels AES									//
	//  -plaintext: Der zu verschl�sselnde Text als SecureString				//
	//  -key: Der Schl�ssel als SecureString									//
	// Gibt den Verschl�sselten Text (Ciphertext) als vector zur�ck				//
	//==========================================================================//
	static vector<uint8_t> aes_encrypt(const SecureString& plaintext,
										const SecureString& key);


	//===========================================================================//
	// Entschl�sselt einen Ciphertext und liefert den urspr�nglichen Klartext	 //
	//  -ciphertext: Der verschl�sselte Text als vector							 //
	//  -key: Der Schl�ssel als SecureString									 //
	//===========================================================================//
	static SecureString aes_decrypt(const vector<uint8_t>& ciphertext,
									const SecureString& key);

};

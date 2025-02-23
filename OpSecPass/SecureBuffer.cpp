#include "SecureBuffer.h"

using namespace std;

// Initialisiert einen sicheren Buffer mit der angegeben Gr��e
SecureBuffer::SecureBuffer(size_t size) : data_(new vector<uint8_t>(size))
{
	
}

// Hebt den Memory-Lock auf und l�scht den Inhalt , um sensible Daten zu entfernen
SecureBuffer::~SecureBuffer()
{
	sodium_munlock(data_->data(), data_->size());
	sodium_memzero(data_->data(), data_->size());
}

// Gibt einen Ptr auf den Speicherbuffer zur�ck
uint8_t* SecureBuffer::data()
{
	return data_->data();
}

// Gibt einen konstanten Zeiger auf den internen Speicherpuffer zur�ck (nur Lesezugriff)
const uint8_t* SecureBuffer::data() const {
	return data_->data();
}


// Liefert die Gr��e des Puffers in Bytes zur�ck
size_t SecureBuffer::size() const 
{
	return data_->size();
}


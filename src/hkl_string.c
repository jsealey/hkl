#include <stddef.h>
#include <assert.h>
#include <string.h>

#include "hkl_string.h"
#include "hkl_alloc.h"

// Test if a byte is likely utf8 encoded
#define hkl_isutf8(c) (((c)&0xC0)!=0x80) 

static const uint32_t HKL_UTF8_MASKS[6] =
{
  0x00000000UL, 0x00003080UL, 0x000E2080UL,
  0x03C82080UL, 0xFA082080UL, 0x82082080UL
};

static uint32_t utf8_nextchar(const char* utf8_data, size_t* index)
{
  uint32_t character = 0;
  size_t character_size = 0;

  do
  {
    character <<= 6;
    character += (char) utf8_data[(*index)++];
    character_size++;

  } while (utf8_data[*index] && !hkl_isutf8(utf8_data[*index]));

  character -= HKL_UTF8_MASKS[character_size - 1];

  return character;
}

static size_t utf8_length(const char* utf8_data)
{
  size_t length = 0;
  size_t index = 0;

  while (utf8_nextchar(utf8_data, &index) != 0)
  {
    length++;
  }

  return length;
}

HklString* hkl_string_new()
{
  HklString* const string = hkl_alloc_object(HklString);
  assert(string != NULL);

  string->utf8_data = NULL;
  string->size = 0;

  return string;
}

HklString* hkl_string_new_from_string(HklString* string) {

  HklString* copy = hkl_string_new();
  hkl_string_copy(copy, string);

  return copy;
}


HklString* hkl_string_new_from_utf8(const char* utf8_data)
{
  HklString* string = hkl_string_new();
  hkl_string_set_utf8(string, utf8_data);

  return string;
}

void hkl_string_set_utf8(HklString* string, const char* utf8_data)
{
  assert(string != NULL);

  size_t size = strlen(utf8_data);
  
  // The input string must be null-terminated
  assert(utf8_data[size] == '\0');

  // If the utf-8 string has a size of zero, then the data should be NULL
  assert((size == 0) == (utf8_data == NULL));

  // Resize the string to accomidate new data
  if (string->size < size)
    string->utf8_data = realloc(string->utf8_data, size + 1);

  string->size = size;
  string->length = utf8_length(utf8_data);

  // If we resized to an empty string, then our new data should be NULL
  assert((string->size == 0) == (string->utf8_data == NULL));

  memcpy(string->utf8_data, utf8_data, size + 1);

  // The string must be NULL terminated by now
  assert(string->utf8_data[size] == '\0');
}

void hkl_string_copy(HklString* string, HklString* src)
{
  assert(string != NULL);
  assert(src != NULL);

  hkl_string_set_utf8(string, src->utf8_data);
}

bool hkl_string_compare(HklString* string1, HklString* string2)
{
  assert(string1 != NULL);
  assert(string2 != NULL);

  // If the lengths and sizes aren't the same, then the can't be the same string internally
  if ((string1->length == string2->length) && (string1->size == string2->size))
  {
    return !strncmp(string1->utf8_data, string2->utf8_data, string1->size);
  }
  else
  {
    return false;
  }
}

/*bool hkl_string_compare_utf8(HklString* string, const char* utf8_data)
{
  assert(string != NULL);
  
}*/

void hkl_string_clear(HklString* string)
{
  assert(string != NULL);

  free(string->utf8_data);

  string->utf8_data = NULL;
  string->size = 0;
  string->length = 0;
}

const char* hkl_string_get_utf8(HklString* string)
{
  assert(string != NULL);

  return string->utf8_data;
}

size_t hkl_string_get_size(HklString* string)
{
  assert(string != NULL);

  return string->size;
}

size_t hkl_string_get_length(HklString* string)
{
  assert(string != NULL);

  return string->length;
}

void hkl_string_free(HklString* string)
{
  assert(string != NULL);

  hkl_string_clear(string);

  free(string);
}
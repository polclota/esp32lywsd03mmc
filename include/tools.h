#include <esp_bt_device.h>
#define placeholder " ";

String centerStr(const String &str, byte new_length)
{
  byte str_length = str.length();

  // if a new length is less or equal length of the original string, returns the original string
  if (new_length <= str_length)
    return str;

  if (str_length > new_length)
    return str.substring(0, new_length);

  String buf;
  byte i, total_rest_length;

  // length of a wrapper of the original string
  total_rest_length = new_length - str_length;

  // write a prefix to buffer
  i = 0;
  buf = "";
  while (i < (total_rest_length / 2))
  {
#ifdef YIELD
    yield();
#endif
    buf = buf + placeholder;
    ++i;
  }

  // write the original string
  buf = buf + str;

  // write a postfix to the buffer
  i += str_length;
  while (i < new_length)
  {
#ifdef YIELD
    yield();
#endif
    buf = buf + placeholder;
    ++i;
  }
  return buf;
}
String printDeviceAddress()
{
  const uint8_t *point = esp_bt_dev_get_address();
  String r;

  for (int i = 0; i < 6; i++)
  {

    char str[3];

    sprintf(str, "%02X", (int)point[i]);
    r += str;

    if (i < 5)
    {
      r += ":";
    }
  }
  return r;
}
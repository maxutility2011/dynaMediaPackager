#include <iostream>
#include <fstream>
#include <sstream>
#include <vector>
#include <stdint.h>
#include <string.h>
#include <stdio.h>

using namespace std;

std::vector<uint8_t> HexToBytes(const std::string& hex) {
  std::vector<uint8_t> bytes;

  for (unsigned int i = 0; i < hex.length(); i += 2) {
    std::string byteString = hex.substr(i, 2);
    uint8_t byte = (uint8_t) strtol(byteString.c_str(), NULL, 16);
    bytes.push_back(byte);
  }

  return bytes;
}

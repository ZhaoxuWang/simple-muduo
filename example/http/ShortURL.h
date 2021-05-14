#pragma once
#include <string>
#include <stdexcept>
#include "md5.h"

int hexstr2int(const std::string& url){

}

class ShortURL{
public:
    static std::string get_shortURL(const std::string& url){
        std::string hex = MD5(url).toStr();
        std::string mixURL = "";
        std::string tmp = "";
        for(int i = 0; i < 4; i++){
            tmp.clear();
            std::string subHex = hex.substr(8 * i, 8);
            int32_t hexint = 0X3FFFFFFF & std::stoul(subHex, 0, 16);
            for(int j = 0; j < 6; j++){
                int32_t idx = 0x0000001F & hexint;
                tmp += alphabet[idx];
                hexint = hexint >> 5;
            }
            mixURL += tmp;
        }
        return mixURL;
    }

private:
    static const std::string alphabet;
    static const std::size_t base;
};

const std::string ShortURL::alphabet("abcdefghijklmnopqrstuvwxyz012345");
const std::size_t ShortURL::base = alphabet.length();
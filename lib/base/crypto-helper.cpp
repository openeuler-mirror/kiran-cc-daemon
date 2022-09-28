/**
 * Copyright (c) 2022 ~ 2023 KylinSec Co., Ltd. 
 * kiran-cc-daemon is licensed under Mulan PSL v2.
 * You can use this software according to the terms and conditions of the Mulan PSL v2. 
 * You may obtain a copy of Mulan PSL v2 at:
 *          http://license.coscl.org.cn/MulanPSL2 
 * THIS SOFTWARE IS PROVIDED ON AN "AS IS" BASIS, WITHOUT WARRANTIES OF ANY KIND, 
 * EITHER EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO NON-INFRINGEMENT, 
 * MERCHANTABILITY OR FIT FOR A PARTICULAR PURPOSE.  
 * See the Mulan PSL v2 for more details.  
 * 
 * Author:     tangjie02 <tangjie02@kylinos.com.cn>
 */

#define CRYPTOPP_ENABLE_NAMESPACE_WEAK 1
#include <cryptopp/base64.h>
#include <cryptopp/des.h>
#include <cryptopp/files.h>
#include <cryptopp/hex.h>
#include <cryptopp/md5.h>
#include <cryptopp/modes.h>
#include <cryptopp/osrng.h>
#include <cryptopp/rsa.h>

#include "lib/base/base.h"
#include "lib/base/crypto-helper.h"

using namespace CryptoPP;

ANONYMOUS_NAMESPACE_BEGIN
#if (CRYPTOPP_USE_AES_GENERATOR)
OFB_Mode<AES>::Encryption s_globalRNG;
#else
NonblockingRng s_globalRNG;
#endif
NAMESPACE_END

RandomNumberGenerator &global_rng()
{
    return dynamic_cast<RandomNumberGenerator &>(s_globalRNG);
}

namespace Kiran
{
CryptoHelper::CryptoHelper()
{
}

CryptoHelper::~CryptoHelper()
{
}

std::string CryptoHelper::md5(const std::string &message)
{
    std::string result;
    Weak::MD5 md5;
    StringSource(message, true, new HashFilter(md5, new HexEncoder(new StringSink(result))));
    return result;
}

std::string CryptoHelper::md5_file(const std::string &filename)
{
    try
    {
        std::string result;
        Weak::MD5 md5;
        FileSource(filename.c_str(), true, new HashFilter(md5, new HexEncoder(new StringSink(result))));
        return result;
    }
    catch (const CryptoPP::Exception &e)
    {
        KLOG_WARNING("%s.", e.what());
        return std::string();
    }
}

std::string CryptoHelper::base64_encrypt(const std::string &message)
{
    try
    {
        std::string result;
        StringSource(message, true, new Base64Encoder(new StringSink(result)));
        return result;
    }
    catch (const CryptoPP::Exception &e)
    {
        KLOG_WARNING("%s.", e.what());
        return std::string();
    }
}

std::string CryptoHelper::base64_decrypt(const std::string &message)
{
    try
    {
        std::string result;
        StringSource(message, true, new Base64Decoder(new StringSink(result)));
        return result;
    }
    catch (const CryptoPP::Exception &e)
    {
        KLOG_WARNING("%s.", e.what());
        return std::string();
    }
}

void CryptoHelper::generate_rsa_key(uint32_t key_length,
                                    std::string &private_key,
                                    std::string &public_key)
{
    try
    {
        RSAES_OAEP_SHA_Decryptor rsa_decryptor(global_rng(), key_length);
        HexEncoder private_sink(new Base64Encoder(new StringSink(private_key)));
        rsa_decryptor.AccessMaterial().Save(private_sink);
        private_sink.MessageEnd();

        RSAES_OAEP_SHA_Encryptor rsa_encryptor(rsa_decryptor);
        HexEncoder public_sink(new Base64Encoder(new StringSink(public_key)));
        rsa_encryptor.AccessMaterial().Save(public_sink);
        public_sink.MessageEnd();
    }
    catch (const CryptoPP::Exception &e)
    {
        KLOG_WARNING("%s.", e.what());
    }
}

std::string CryptoHelper::rsa_encrypt(const std::string &public_key,
                                      const std::string &message)
{
    RETURN_VAL_IF_TRUE(message.empty(), std::string());

    try
    {
        RandomPool random_pool;
        StringSource public_source(public_key, true, new Base64Decoder(new HexDecoder));
        RSAES_OAEP_SHA_Encryptor rsa_encryptor(public_source);

        if (message.size() > rsa_encryptor.FixedMaxPlaintextLength())
        {
            KLOG_WARNING("The length(%d) of message is greater than the value(%d) which FixedMaxPlaintextLength return.",
                         message.size(),
                         rsa_encryptor.FixedMaxPlaintextLength());
            return std::string();
        }

        std::string result;
        StringSource(message, true, new PK_EncryptorFilter(random_pool, rsa_encryptor, new HexEncoder(new StringSink(result))));
        return result;
    }
    catch (const CryptoPP::Exception &e)
    {
        KLOG_WARNING("%s.", e.what());
        return std::string();
    }
}

std::string CryptoHelper::rsa_decrypt(const std::string &private_key, const std::string &ciphertext)
{
    RETURN_VAL_IF_TRUE(ciphertext.empty(), std::string());

    try
    {
        RandomPool random_pool;
        StringSource private_source(private_key, true, new Base64Decoder(new HexDecoder));
        RSAES_OAEP_SHA_Decryptor priv(private_source);

        // 需要先HexDecoder后才能比较大小
        // if (ciphertext.size() > priv.FixedCiphertextLength())
        // {
        //     KLOG_WARNING("The length(%d) of message is greater than the value(%d) which FixedCiphertextLength return.",
        //                 ciphertext.size(),
        //                 priv.FixedCiphertextLength());
        //     return std::string();
        // }

        std::string result;
        StringSource(ciphertext, true, new HexDecoder(new PK_DecryptorFilter(random_pool, priv, new StringSink(result))));
        return result;
    }
    catch (const CryptoPP::Exception &e)
    {
        KLOG_WARNING("%s.", e.what());
        return std::string();
    }
}

bool CryptoHelper::rsa_sign_file(const std::string &private_filename,
                                 const std::string &message_filename,
                                 const std::string &signature_filename)
{
    try
    {
        FileSource priv_file(private_filename.c_str(), true, new Base64Decoder(new HexDecoder));

        RSASS<PKCS1v15, SHA1>::Signer priv(priv_file);
        FileSource(message_filename.c_str(), true, new SignerFilter(global_rng(), priv, new HexEncoder(new FileSink(signature_filename.c_str()))));
        return true;
    }
    catch (const CryptoPP::Exception &e)
    {
        KLOG_WARNING("%s.", e.what());
        return false;
    }
}

bool CryptoHelper::rsa_verify_file(const std::string &public_filename,
                                   const std::string &message_filename,
                                   const std::string &signature_filename)
{
    try
    {
        FileSource pub_file(public_filename.c_str(), true, new Base64Decoder(new HexDecoder));
        RSASS<PKCS1v15, SHA1>::Verifier pub(pub_file);

        FileSource signature_file(signature_filename.c_str(), true, new HexDecoder);
        RETURN_VAL_IF_FALSE(signature_file.MaxRetrievable() == pub.SignatureLength(), false);

        SecByteBlock signature(pub.SignatureLength());
        signature_file.Get(signature, signature.size());

        SignatureVerificationFilter *verifier_filter = new SignatureVerificationFilter(pub);
        verifier_filter->Put(signature, pub.SignatureLength());
        FileSource(message_filename.c_str(), true, verifier_filter);

        return verifier_filter->GetLastResult();
    }
    catch (const CryptoPP::Exception &e)
    {
        KLOG_WARNING("%s.", e.what());
        return false;
    }
}

std::string CryptoHelper::des_encrypt(const std::string &message, const std::string &key)
{
    try
    {
        std::string result;
        ECB_Mode<DES>::Encryption encoder;
        // 这里的key长度必须为8
        encoder.SetKey((const byte *)key.c_str(), key.length());
        // auto key = get_des_key();
        // encoder.SetKey(key, key.size());
        StringSource(message, true, new StreamTransformationFilter(encoder, new Base64Encoder(new StringSink(result))));
        return result;
    }
    catch (const CryptoPP::Exception &e)
    {
        KLOG_WARNING("%s.", e.what());
        return std::string();
    }
}

std::string CryptoHelper::des_decrypt(const std::string &message, const std::string &key)
{
    try
    {
        std::string result;
        ECB_Mode<DES>::Decryption decoder;
        decoder.SetKey((const byte *)key.c_str(), key.length());
        StringSource(message, true, new Base64Decoder(new StreamTransformationFilter(decoder, new StringSink(result))));
        return result;
    }
    catch (const CryptoPP::Exception &e)
    {
        KLOG_WARNING("%s.", e.what());
        return std::string();
    }
}

}  // namespace Kiran

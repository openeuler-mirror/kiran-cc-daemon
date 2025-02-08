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

void CryptoHelper::generateRSAKey(uint32_t keyLength, QString &privateKey, QString &publicKey)
{
    std::string stlPrivateKey;
    std::string stlPublicKey;
    try
    {
        RSAES_OAEP_SHA_Decryptor rsa_decryptor(global_rng(), keyLength);
        HexEncoder private_sink(new Base64Encoder(new StringSink(stlPrivateKey)));
        rsa_decryptor.AccessMaterial().Save(private_sink);
        private_sink.MessageEnd();

        RSAES_OAEP_SHA_Encryptor rsa_encryptor(rsa_decryptor);
        HexEncoder public_sink(new Base64Encoder(new StringSink(stlPublicKey)));
        rsa_encryptor.AccessMaterial().Save(public_sink);
        public_sink.MessageEnd();

        privateKey = QString::fromStdString(stlPrivateKey);
        publicKey = QString::fromStdString(stlPublicKey);
    }
    catch (const CryptoPP::Exception &e)
    {
        KLOG_WARNING("%s.", e.what());
        return;
    }
}

QString CryptoHelper::rsaEncrypt(const QString &publicKey, const QString &message)
{
    RETURN_VAL_IF_TRUE(message.isEmpty(), QString());

    try
    {
        RandomPool random_pool;
        StringSource public_source(publicKey.toStdString(), true, new Base64Decoder(new HexDecoder));
        RSAES_OAEP_SHA_Encryptor rsa_encryptor(public_source);

        if (message.size() > int(rsa_encryptor.FixedMaxPlaintextLength()))
        {
            KLOG_WARNING() << "The length(" << message.size()
                           << ") of message is greater than the value("
                           << rsa_encryptor.FixedMaxPlaintextLength()
                           << ") which FixedMaxPlaintextLength return.";
            return QString();
        }

        std::string result;
        StringSource(message.toStdString(), true, new PK_EncryptorFilter(random_pool, rsa_encryptor, new HexEncoder(new StringSink(result))));
        return QString::fromStdString(result);
    }
    catch (const CryptoPP::Exception &e)
    {
        KLOG_WARNING("%s.", e.what());
        return QString();
    }
}

QString CryptoHelper::rsaDecrypt(const QString &privateKey, const QString &ciphertext)
{
    RETURN_VAL_IF_TRUE(ciphertext.isEmpty(), QString());

    try
    {
        RandomPool random_pool;
        StringSource private_source(privateKey.toStdString(), true, new Base64Decoder(new HexDecoder));
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
        StringSource(ciphertext.toStdString(), true, new HexDecoder(new PK_DecryptorFilter(random_pool, priv, new StringSink(result))));
        return QString::fromStdString(result);
    }
    catch (const CryptoPP::Exception &e)
    {
        KLOG_WARNING("%s.", e.what());
        return QString();
    }
}

}  // namespace Kiran

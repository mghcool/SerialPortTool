#include "crccheck.h"
#include <QByteArray>
#include <QDebug>

const ParamModel CrcCheck ::modelList[] = {
    //                     宽度  多项式       初始值       结果异或值   输入反转 输出反转
    { "CRC-4/ITU",          4,  0x03,       0x00,       0x00,       true,  true  },
    { "CRC-5/EPC",          5,  0x09,       0x09,       0x00,       false, false },
    { "CRC-5/ITU",          5,  0x15,       0x00,       0x00,       true,  true  },
    { "CRC-5/USB",          5,  0x05,       0x1F,       0x1F,       true,  true  },
    { "CRC-6/ITU",          6,  0x03,       0x00,       0x00,       true,  true  },
    { "CRC-7/MMC",          7,  0x09,       0x00,       0x00,       false, false },
    { "CRC-8",              8,  0x07,       0x00,       0x00,       false, false },
    { "CRC-8/ITU",          8,  0x07,       0x00,       0x55,       false, false },
    { "CRC-8/ROHC",         8,  0x07,       0xFF,       0x00,       true,  true  },
    { "CRC-8/MAXIM",        8,  0x31,       0x00,       0x00,       true,  true  },
    { "CRC-16/IBM",         16, 0x8005,     0x0000,     0x0000,     true,  true  },
    { "CRC-16/MAXIM",       16, 0x8005,     0x0000,     0xFFFF,     true,  true  },
    { "CRC-16/USB",         16, 0x8005,     0xFFFF,     0xFFFF,     true,  true  },
    { "CRC-16/MODBUS",      16, 0x8005,     0xFFFF,     0x0000,     true,  true  },
    { "CRC-16/CCITT",       16, 0x1021,     0x0000,     0x0000,     true,  true  },
    { "CRC-16/CCITT-FALSE", 16, 0x1021,     0xFFFF,     0x0000,     false, false },
    { "CRC-16/X25",         16, 0x1021,     0xFFFF,     0xFFFF,     true,  true  },
    { "CRC-16/XMODEM",      16, 0x1021,     0x0000,     0x0000,     false, false },
    { "CRC-16/DNP",         16, 0x3D65,     0x0000,     0xFFFF,     true,  true  },
    { "CRC-32",             32, 0x04C11DB7, 0xFFFFFFFF, 0xFFFFFFFF, true,  true  },
    { "CRC-32/MPEG-2",      32, 0x04C11DB7, 0xFFFFFFFF, 0x00000000, false, false }
};

const int CrcCheck ::modelListSize = sizeof(CrcCheck ::modelList)/sizeof(CrcCheck ::modelList[0]);

CrcCheck ::CrcCheck ()
{

}

/**
 * @brief 二进制位转换
 * @param width 数字宽度
 * @param val   要转的值
 * @return      转换后的值
 */
quint32 CrcCheck::reflect(quint8 width, quint32 val)
{
    quint32 valRet = 0;
    for(int i = 0; i < width; i++)
    {
        valRet <<= 1;
        valRet |= val & 1;
        val >>= 1;
    }
    return valRet;
}

/**
 * @brief CrcCheck::computeCrcVal
 * @param buf
 * @param index
 * @return
 */
quint32 CrcCheck::computeCrcVal(QByteArray buf, int index)
{
    quint16 crcVal = this->modelList[index].init;
    quint16 poly = this->modelList[index].poly;
    //quint16 xorout = this->modelList[index].xorout;
    bool refin = this->modelList[index].refin;
    //bool refout = this->modelList[index].refout;

    if(refin)
    {
        poly = reflect(16, poly);
        for (int x = 0; x < buf.size(); x++)
        {
            crcVal ^= buf.at(x);
            for (int y = 0; y < 8; y++)
            {
                if (crcVal & 0x0001)
                {
                    crcVal >>= 1;
                    crcVal ^= poly;
                }
                else
                    crcVal >>= 1;
            }
        }
    }
    else
    {
        for (int x = 0; x < buf.size(); x++)
        {
            crcVal ^= buf.at(x);
            for (int y = 0; y < 8; y++)
            {
                if (crcVal & 0x0001)
                {
                    crcVal <<= 1;
                    crcVal ^= poly;
                }
                else
                    crcVal <<= 1;
            }
        }
    }

    return (quint32)crcVal;
}

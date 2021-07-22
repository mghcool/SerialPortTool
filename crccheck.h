#ifndef CRC_H
#define CRC_H

#include <QtGlobal>
#include <QString>

// 参数模型结构类型
typedef struct
{
    QString name;   //名字
    quint8 width;   //宽度
    quint32 poly;   //多项式
    quint32 init;   //初始值
    quint32 xorout; //结果异或值
    bool refin;     //输入反转
    bool refout;    //输出反转
} ParamModel;

class CrcCheck 
{
public:
    static const ParamModel modelList[];
    static const int modelListSize;

    CrcCheck ();
    quint32 computeCrcVal(QByteArray buf, int index);

private:
    quint32 reflect(quint8 width, quint32 val);
};

#endif // CRC_H

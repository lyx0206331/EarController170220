
#include <string.h>
#include <stdio.h>
#include <jni.h>
#include <android/log.h>
#include <math.h>

#define ResultBufferSize 32 //should be big enough
#define SampleRate 32000

#if SampleRate==32000
#define AvgQueueLength  16  // should be power of 2. 16 is just fine
#define HalfPeriod_1K        16
#define HalfPeriod_0_5_K   32
#define HalfPeriod_Least 10
#define ErrorThreshold    5  // it shold be set reasonably. Better bigger than 4, smaller than 7
#define LowPassFilter      16
#else
#define AvgQueueLength  32  // should be power of 2. 32 is just fine
#define HalfPeriod_1K        22
#define HalfPeriod_0_5_K   44
#define HalfPeriod_Least 12
#define ErrorThreshold    6  // it shold be set reasonably. Better bigger than 4, smaller than 7
#define LowPassFilter      32
#endif

enum DecodingState {
    DS_Leading1 = 0,
    DS_Leading2,
    DS_StartBit,
    DS_DataBit,
};

#define resetDecoding() do {\
    bufCount = 0;\
    periodIndex = 0;\
    bit1Counter = 0;\
    ds = DS_Leading1;\
    }while(0)

struct DecodedResult {
    int count;
    struct {
        int globalKeySeq;
        int localKeySeq;
        int keyCode;
        int isDown;
        int original;
    }keyEventQueue[ResultBufferSize];
};

static int fskDecode(int d, struct DecodedResult *pdr) {
    static int lastDelta = 0, lastAbs = 0, delta2 =0, lastDelta2 = 0;
    static int buf[66], bufCount = 0;
    static int periodBuf[200], periodIndex =0;
    static int bitIndex = 0, bit1Counter = 0;
    static enum DecodingState ds = DS_Leading1;
    static int data = 0, chkSum = 0;
    int i = 0;
    int positiveCounter, negativeCounter;
    int period;
    int newbit = -1;
    static int threshold = 0;
    static int debugIndex = 0;
    static int leading2Counter = 0;
    int ret = -1;
    int delta = 0;
    static int avgQueue[AvgQueueLength], avgSum = 0, avgIndex = AvgQueueLength;
    static int zeroThreshold = 0;
    int peak = 0;
    int zeroCrossed = 0, sum = 0;

    debugIndex++;

    avgSum -= avgQueue[avgIndex&(AvgQueueLength-1)];
    avgQueue[avgIndex&(AvgQueueLength-1)] = d;
    avgSum+=d;
    avgIndex++;
//    qDebug() << lastAbs << "," << d;
    d = avgSum/AvgQueueLength;
    delta = (d-lastAbs)*128;
    lastAbs = d;

    delta2 = (delta - lastDelta);
    delta2= (lastDelta2*(LowPassFilter-1)+delta2)/LowPassFilter;

    //    qDebug() << d  << "," << delta << "," << delta2;
//    qDebug() << delta2;

    if(lastDelta2*delta2 <=0 && lastDelta2 != 0) {
        zeroCrossed = 1;
        if(ds >= DS_Leading1) {
            if(bufCount <= HalfPeriod_Least) {
                sum  = 0;
                for(i = 0; i < bufCount; i++) {
                    sum += buf[i];
                }
                if(sum*delta2<0 && delta2<abs(zeroThreshold)) {
                    zeroCrossed = 0;
                    delta2 = -delta2;
                }
            }
        }
    }

    //过零检测
    if(zeroCrossed) { /*避免上一次为0导致重复检测到过零*/

        //检查上一个半周期的波形是正还是负
        //并找到峰值
        if(bufCount >= HalfPeriod_Least) {
            positiveCounter =0;
            negativeCounter = 0;
            for(i = 0; i< bufCount; i++) {
                if(buf[i] > threshold) {
                    positiveCounter ++;
                } else if(buf[i] < threshold) {
                    negativeCounter++;
                }
            }
            //FIXME.除检查波形外，还应考虑检查包络
            if(positiveCounter >= bufCount-2) {
                //几乎全为正
                if(periodIndex != 0) {  //相位偏移了180°，从负半周开始统计
                    periodBuf[periodIndex++] = bufCount>>1;
                }
            } else if(negativeCounter >= bufCount -2) {
                periodBuf[periodIndex++] = 0-(bufCount>>1);
            } else {
                resetDecoding();
            }

            if(periodIndex >= 200) {
                //不可能有这么多码元
                resetDecoding();
            }

            //每偶数个半周期解码一次
            if(periodIndex && ((periodIndex & 0x01) ==0)) {
                if(periodBuf[periodIndex-1] <= 0 || periodBuf[periodIndex-2] >= 0) {
                    resetDecoding();        //必须是前负后正成对出现
                } else {
                    period = 0+periodBuf[periodIndex-1]-periodBuf[periodIndex-2];
                    if( period >= HalfPeriod_1K-ErrorThreshold && period <= HalfPeriod_1K+ErrorThreshold) {
                        bit1Counter++;
                        if(bit1Counter >= 2) {      //两个1KHz正弦波才算作一个1
                            newbit=1;
                            bit1Counter = 0;
                        }
                    } else if(period >= HalfPeriod_0_5_K-(ErrorThreshold<<1) &&
                              period <= HalfPeriod_0_5_K+(ErrorThreshold<<1)) {
                        if(bit1Counter) {       //两种半周期交替出现
                            if(ds == DS_StartBit) { //改状态下可以接受，其他状态下需重新解码
                                newbit = 0;
                                bit1Counter = 0;
                            } else {
                                resetDecoding();
                            }
                        } else {
                            newbit = 0;
                        }
                    } else {
                        resetDecoding();    //周期不对
                    }
                }
            }

            if(newbit ==0 || newbit ==1) {
                switch (ds) {
                case DS_Leading1:
                    if(newbit == 1) {       //检测到第一个1；再等待至少4个1
                        ds = DS_Leading2;
                        leading2Counter = 0;
                    }
                    break;
                case DS_Leading2:
                    if(newbit == 1) {
                        leading2Counter++;
                        if(leading2Counter == 4) {   //检测到第5个1；开始等待一个0作为起始位
                            ds = DS_StartBit;
                            peak = 0;
                            zeroThreshold = 0;
                            for(i = 0; i < bufCount; i++) {
                                if(peak < buf[i])
                                    peak = buf[i];
                            }
                            zeroThreshold = peak/20;
                            //                            qDebug() << zeroThreshold;
                        }
                    } else {
                        ds = DS_Leading1;
                    }
                    break;
                case DS_StartBit:
                    if(newbit == 0) {   //检测到起始位；开始接受数据
                        ds = DS_DataBit;
                        bitIndex = 0;
                        data = 0;
                        chkSum = 0;
                    }
                    break;
                case DS_DataBit:
                    //2 bit global flag; 2 bit local flag; 1 bit up/down flag; 3 bit key value;3 bit chksum;
                    if(bitIndex < 2+2+1+3+3) {
                        if(newbit)
                            data |= (1<< ((2+2+1+3+3)-1-bitIndex));
                        bitIndex++;
                        if(bitIndex >= 1 && bitIndex <= 8) {
                            chkSum += newbit;
                        }

                        if(bitIndex == (2+2+1+3+3)) {
                            ds = DS_Leading1;
                            if(chkSum == (data & 0x07)) {
                                if(pdr) {
                                    //去掉chksum的数据
                                    if(pdr->count<ResultBufferSize) {
                                        pdr->keyEventQueue[pdr->count].original = data >> 3;
                                        pdr->keyEventQueue[pdr->count].keyCode = (data>>3)&0x07;
                                        pdr->keyEventQueue[pdr->count].localKeySeq = (data>>7)&0x03;
                                        pdr->keyEventQueue[pdr->count].isDown = (data>>6)&0x01;
                                        pdr->keyEventQueue[pdr->count].globalKeySeq = (data>>9)&0x03;
                                        pdr->count++;
                                    }
                                }
                                ret = data >> 3;
                            }
                            resetDecoding();
                        }
                    }
                    break;
                default:
                    break;
                }
            }
        } else {
            resetDecoding();
        }

        bufCount = 0;
    }

    //44.1kHz采样率下的500Hz正弦波的半周期为约44个点，全周期为约88个点
    //实际波形半周期不准，取66作为上限
    //过多的半周期数据.解码复位
    if(bufCount < 66) {
        buf[bufCount++] = delta2;
    } else {
        resetDecoding();
    }

    lastDelta = delta;
    lastDelta2 = delta2;

    return ret;
}

jintArray Java_com_adrian_earctrllib_TunnerThread_processSampleData(JNIEnv*  env,jobject thiz,jbyteArray sample,jint sampleLength){
    jsize arrayLength = (*env)->GetArrayLength(env,sample);
    jbyte* localSample = (*env)->GetByteArrayElements(env,sample,0);
    double padSample[sampleLength/2];
    int len = sampleLength/2;
    int res = -1;
    int i,j;
    jintArray array = NULL;
    struct DecodedResult dr;
    int resArray[ResultBufferSize];
    
    if(sampleLength > arrayLength) {
        sampleLength = arrayLength;
    }
    
    // big to little endian and hanning
    for(i=0;i<sampleLength;i+=2){
        padSample[i >> 1] =  (short)((localSample[i]&0xFF) | ((localSample[i+1]&0xFF) << 8));
    }

    dr.count = 0;
    for (i = 0; i < len; ++i) {
        fskDecode(padSample[i], &dr);
    }
    
    if(dr.count != 0) {
        array =  (*env)->NewIntArray(env, dr.count);
        for (j=0; j<dr.count; j++) {
            resArray[j] = dr.keyEventQueue[j].original;
        }
        (*env)->SetIntArrayRegion(env,array, 0, dr.count, resArray);
    }
    
    (*env)->ReleaseByteArrayElements(env,sample,localSample,0);
    return array;
}


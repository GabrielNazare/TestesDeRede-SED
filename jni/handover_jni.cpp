
#include <jni.h>
#include "../simulator/include/SignalFilter.h"

extern "C" {

JNIEXPORT jdouble JNICALL
Java_com_handover_jni_HandoverJNI_computeFilteredRsrp(
    JNIEnv*      javaEnvironment,
    jobject      callerObject,
    jdoubleArray rsrpSamplesArray,
    jint         windowSize)
{
    jsize sampleCount = javaEnvironment->GetArrayLength(rsrpSamplesArray);
    jdouble* nativeSamples = javaEnvironment->GetDoubleArrayElements(rsrpSamplesArray, nullptr);

    SignalFilter filter(static_cast<size_t>(windowSize));

    for (jsize i = 0; i < sampleCount; ++i) {
        filter.addSample(static_cast<double>(nativeSamples[i]));
    }

    javaEnvironment->ReleaseDoubleArrayElements(rsrpSamplesArray, nativeSamples, JNI_ABORT);

    return static_cast<jdouble>(filter.getFilteredRssi());
}

JNIEXPORT jboolean JNICALL
Java_com_handover_jni_HandoverJNI_isA3EventConditionMet(
    JNIEnv* javaEnvironment,
    jobject callerObject,
    jdouble neighborRsrpDbm,
    jdouble servingRsrpDbm,
    jdouble hysteresisDb)
{
    bool a3ConditionSatisfied = (neighborRsrpDbm - hysteresisDb) > servingRsrpDbm;
    return static_cast<jboolean>(a3ConditionSatisfied);
}

}

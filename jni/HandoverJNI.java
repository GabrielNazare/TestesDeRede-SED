package com.handover.jni;

public class HandoverJNI {

    static {
        System.loadLibrary("handover_core");
    }

    public native double computeFilteredRsrp(double[] rsrpSamples, int windowSize);

    public native boolean isA3EventConditionMet(
        double neighborRsrpDbm,
        double servingRsrpDbm,
        double hysteresisDb
    );

    public static void demonstrateAndroidTelephonyFlow() {
        HandoverJNI jniInterface = new HandoverJNI();

        double[] rawRsrpReadings = { -72.0, -68.0, -75.0, -71.0, -69.0 };

        double filteredRsrp = jniInterface.computeFilteredRsrp(rawRsrpReadings, 5);
        System.out.println("[Android] Filtered RSRP: " + filteredRsrp + " dBm");

        double neighborRsrp = -65.0;
        double servingRsrp  = filteredRsrp;
        double hysteresis   = 3.0;

        boolean a3Met = jniInterface.isA3EventConditionMet(neighborRsrp, servingRsrp, hysteresis);
        if (a3Met) {
            System.out.println("[Android] A3 Event detected! Initiating handover procedure...");
        } else {
            System.out.println("[Android] Signal stable. No handover needed.");
        }
    }

    public static void main(String[] args) {
        demonstrateAndroidTelephonyFlow();
    }
}

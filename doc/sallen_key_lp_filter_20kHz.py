#*******************************************************************************
#
# File: sallen_key_lp_filter_20khz.py
# Author: Ryan Clarke (kj6msg@arrl.net)
#
# 4th Order Sallen-Key Unity Gain Butterworth Low-Pass Filter
#
# History:
#
# v1.0 - First release
#
#*******************************************************************************

import math
from scipy import signal
import matplotlib.pyplot as plt

fc = 20000.     # sample rate (Hz)
C = 3300e-12    # base capacitor

# 4th order Sallen-Key Butterworth filter Q values
Q1 = 0.5412
Q2 = 1.3065

# Sallen-Key ratio components (equal resistors)
m = 1
n1 = 4 * Q1 * Q1
n2 = 4 * Q2 * Q2

# Sallen-Key capacitor and resistor computations
C1_1 = C
C2_1 = n1 * C
#C2_1 = 3900e-12

C1_2 = C
C2_2 = n2 * C
#C2_2 = 0.022e-6

R_1 = 1. / (2 * math.pi * fc * C * math.sqrt(m * n1))
#R_1 = 2.2e3
R_2 = 1. / (2 * math.pi * fc * C * math.sqrt(m * n2))
#R_2 = 910
#R_2 = 1000

print("4th Order Sallen-Key Unity Gain Butterworth Low-Pass Filter")

print()
print("Stage One:")
print("Fc:", fc)
print("C1:", C1_1)
print("C2:", C2_1)
print("R1:", R_1)
print("R2:", R_1)

print()
print("Stage Two:")
print("Fc:", fc)
print("C1:", C1_2)
print("C2:", C2_2)
print("R1:", R_2)
print("R2:", R_2)

# plot the frequency response of each stage and the complete filter
num1 = [1]
den1 = [C1_1 * C2_1 * R_1 * R_1, C1_1 * (R_1 + R_1), 1]

num2 = [1]
den2 = [C1_2 * C2_2 * R_2 * R_2, C1_2 * (R_2 + R_2), 1]

s1 = signal.lti(num1, den1)
s2 = signal.lti(num2, den2)

w1, mag1, phase1 = s1.bode()
w2, mag2, phase2 = s2.bode()

w1 = w1 / (2 * math.pi)
w2 = w2 / (2 * math.pi)

plt.figure()
plt.semilogx(w1, mag1)
plt.semilogx(w2, mag2)
plt.semilogx(w1, mag1+mag2)
plt.legend(['Stage 1', 'Stage 2', 'Combined'])
plt.grid(True, which='both')
plt.title('4th Order Sallen-Key Unity Gain Butterworth Low-Pass Filter')
plt.ylabel('Gain (dBV)')
plt.xlabel('Frequency (Hz)')
plt.show()

import numpy as np
import matplotlib.pyplot as plt

# Define component values
R1 = 4.7e3  # 4.7k Ohm
R2 = 1e3    # 1k Ohm
C = 3.3e-9  # 3.3nF

# Define frequency range (log scale)
freqs = np.logspace(2, 6, 1000)  # 100 Hz to 1 MHz
omega = 2 * np.pi * freqs

# Define capacitor impedance
Z_C = -1j / (omega * C)

# Your derived transfer function: H(f) = -R2 * (R1 + Zc)
H_f = -R2 * (R1 + Z_C)

# Plotting
fig, ax1 = plt.subplots()
ax2 = ax1.twinx()

# Magnitude in dB
ax1.plot(freqs, 20 * np.log10(np.abs(H_f)), 'b', label='Magnitude (dB)')
ax1.set_xlabel("Frequency (Hz)")
ax1.set_ylabel("Magnitude (dB)", color='b')
ax1.set_xscale("log")
ax1.tick_params(axis='y', labelcolor='b')

# Phase in degrees
ax2.plot(freqs, np.angle(H_f, deg=True), 'r', label='Phase (degrees)')
ax2.set_ylabel("Phase (degrees)", color='r')
ax2.tick_params(axis='y', labelcolor='r')

plt.title("Transfer Function: H(f) = -R2(R1 + Zc)")
plt.grid(True, which="both", linestyle="--", linewidth=0.5)
plt.show()
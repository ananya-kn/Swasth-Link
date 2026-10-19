"""Ideal-case drop load: peak deceleration = v^2 / (2 d) for a bumper that compresses by d at constant force.
Real impacts are worse. Confirm with a physical 1.2 m drop test (six faces, concrete)."""
import math
H_DROP, MASS_KG, G = 1.2, 0.25, 9.81
v = math.sqrt(2 * G * H_DROP)
print(f"speed {v:.2f} m/s, energy {MASS_KG * G * H_DROP:.2f} J")
for d_mm in (1, 2, 3, 5, 8, 10, 12):
    print(f"bumper stroke {d_mm:>2} mm -> {v * v / (2 * d_mm / 1000) / G:>6.0f} g")

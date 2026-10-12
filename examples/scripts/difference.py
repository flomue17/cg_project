import numpy as np
import matplotlib.pyplot as plt
from PIL import Image

# Zwei Bilder laden
image1 = Image.open('../weighting.png')
image2 = Image.open('../buckets.png')

# Sicherstellen, dass beide Bilder im RGB-Modus sind (falls sie unterschiedliche Kanäle haben)
image1 = image1.convert('RGB')
image2 = image2.convert('RGB')

# Umwandeln der Bilder in Numpy-Arrays
image_data1 = np.array(image1).astype(np.float32)  # Umwandeln in 32-Bit Float
image_data2 = np.array(image2).astype(np.float32)  # Umwandeln in 32-Bit Float

# Überprüfen, ob die Bilder die gleiche Größe haben
if image_data1.shape != image_data2.shape:
    raise ValueError("Die Bilder müssen die gleiche Größe haben!")

# Berechnen der absoluten Differenz der Farbwerte
difference = np.abs(image_data1 - image_data2)

# Wenn die Differenzwerte zu groß sind (z.B. Werte zwischen 0 und 255), kann die Anzeige schwierig sein. Wir normalisieren sie:
normalized_difference = (difference - np.min(difference)) / (np.max(difference) - np.min(difference))

# Umkehren der Normalisierung: Schwarz für große Unterschiede, Weiß für keine Unterschiede
reversed_difference = 1 - normalized_difference

# Visualisieren der Differenz
plt.imshow(reversed_difference, cmap='hot')  # Hot colormap für Heatmap-Darstellung
plt.colorbar()  # Farbskala anzeigen
plt.title("Difference between masks")

plt.savefig('./outputs/diff_masks.png', dpi=300)  # Speichert das Bild mit 300 dpi Qualität

plt.show()

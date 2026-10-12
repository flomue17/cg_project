from PIL import Image
import numpy as np
import matplotlib.pyplot as plt
from matplotlib import ticker

# PNG-Datei laden
image = Image.open('../confidence.png')

# Umwandeln in ein Numpy-Array (das Bild hat nur einen Kanal)
image_data = np.array(image)

# Überprüfen der Form des Arrays (sollte (1024, 2048) sein)
print("Form des Bildes:", image_data.shape)

# Schritt 2: Normalisierung der Zählerwerte
min_value = np.min(image_data)  # Minimaler Zählerwert
max_value = np.max(image_data)  # Maximaler Zählerwert

print(max_value, min_value)
# Normalisierung der Werte in den Bereich [0, 1]
normalized_data = (image_data - min_value) / (max_value - min_value)

# Schritt 3: Erstellen der Heatmap
fig, ax = plt.subplots()
cax = ax.imshow(normalized_data, cmap='inferno')  # 'inferno' ist eine Farbkarte für Heatmaps

# Colorbar mit den absoluten Zählerwerten (Skalierung zurück auf die originalen Werte)
cbar = fig.colorbar(cax, ax=ax)

# Die Farbskala in die originalen Zählerwerte zurückskalieren
cbar.set_ticks([0, 0.25, 0.5, 0.75, 1])  # Ticks auf der Colorbar
cbar.set_ticklabels([min_value, min_value + 0.25 * (max_value - min_value),
                     min_value + 0.5 * (max_value - min_value),
                     min_value + 0.75 * (max_value - min_value),
                     max_value])  # Die Labels anpassen, um die tatsächlichen Zählerwerte anzuzeigen

# Optional: Titel und Achsenbeschriftungen
plt.title("Confidence")

# Speichern des Heatmap-Bildes als PNG
plt.savefig('./outputs/confidence_heatmap.png', dpi=300)  # Speichert das Bild mit 300 dpi Qualität

# Optional: Das Bild auch anzeigen
plt.show()

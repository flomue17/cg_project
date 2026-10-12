from PIL import Image
import numpy as np
import matplotlib.pyplot as plt

# PNG-Datei laden
image = Image.open('../deviation.png')

# Umwandeln in ein Numpy-Array (das Bild hat möglicherweise 4 Kanäle: RGBA)
image_data = np.array(image)

# Überprüfen der Form des Arrays
print("Form des Bildes:", image_data.shape)

# Falls das Bild 4 Kanäle hat (RGBA), entfernen wir den Alpha-Kanal
if image_data.shape[-1] == 4:
    image_data = image_data[:, :, :3]  # Entferne den Alpha-Kanal

# Schritt 2: Normalisierung der Zählerwerte
min_value = np.min(image_data)  # Minimaler Zählerwert
max_value = np.max(image_data)  # Maximaler Zählerwert

print("Max:", max_value, "Min:", min_value)

# Normalisierung der Werte in den Bereich [0, 1]
normalized_data = (image_data - min_value) / (max_value - min_value)

# Schritt 3: Erstellen der RGB-Daten
# Wiederhole normalized_data entlang der dritten Dimension (für R, G, B)
rgb_data = np.repeat(normalized_data[:, :, np.newaxis], 3, axis=2)

# Schritt 4: Erstellen der Heatmap im RGB-Bereich
fig, ax = plt.subplots()
cax = ax.imshow(rgb_data)  # RGB Heatmap anzeigen

# Optional: Farbskala hinzufügen
# Hier wird eine Farbskala hinzugefügt, aber da es sich um RGB handelt, sind die Ticks meist nicht nötig
cbar = fig.colorbar(cax, ax=ax)

# Optional: Titel und Achsenbeschriftungen
plt.title("Deviation - RGB Visualisierung")

# Speichern des RGB Heatmap-Bildes als PNG
plt.savefig('./outputs/deviation_rgb_map.png', dpi=300)  # Speichert das Bild mit 300 dpi Qualität

# Optional: Das Bild auch anzeigen
plt.show()

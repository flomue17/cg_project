import os
from sklearn.cluster import KMeans
import numpy as np
import matplotlib.pyplot as plt
import matplotlib.image as mpimg
from PIL import Image


# Funktion zum Laden jedes 5. Bildes aus einem Ordner
def load_every_fifth_image(directory):
    image_files = sorted([f for f in os.listdir(directory) if f.endswith(('jpg', 'jpeg', 'png'))])
    selected_images = []

    for i in range(0, len(image_files), 5):  # Jedes 5. Bild
        image_path = os.path.join(directory, image_files[i])
        img = mpimg.imread(image_path)
        if img.dtype == np.float32 or img.dtype == np.float64:
            img = (img * 255).astype(np.uint8)
        selected_images.append(img)

    return selected_images


# Ordner mit den Bildern
image_folder = "../scenes/tt_francis/images/"

# Lade jedes 5. Bild aus dem Ordner
images = load_every_fifth_image(image_folder)

# Die Bilder nebeneinander aneinanderhängen
combined_image = np.hstack(images)  # Alle Bilder horizontal aneinanderhängen

# Hole Bildgröße
w, h, d = combined_image.shape

# Reshape des Bildes zu einer Liste von Pixeln
pixel = np.reshape(combined_image, (w * h, d))

# Anzahl der Farben
n_colors = 8

# KMeans Clustering durchführen
model = KMeans(n_clusters=n_colors, random_state=42).fit(pixel)

# Clusterzentren als Farbpalette extrahieren
colour_palette = model.cluster_centers_ / 255.0  # Werte im Bereich [0, 1]

# Bild mit den extrahierten Farben erstellen
w, h = 8, 1  # Bildgröße für die Farbpalette
data = np.zeros((h, w, 4), dtype=np.float32)  # Bild mit 4 Kanälen für RGBA32F

# Farben ins Bild speichern
for i in range(n_colors):
    color = colour_palette[i]  # RGB-Werte im Bereich [0, 1]
    # Speichern der Farben als Float32 im Bereich [0, 1]
    data[0, i] = [color[0], color[1], color[2], 1.0]

# Erstelle ein Bild aus den gespeicherten Farben
img = Image.fromarray((data * 255).astype(np.uint8), 'RGBA')  # Umwandeln für die Anzeige
img.save('k-means.png')
img.show()

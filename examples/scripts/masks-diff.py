# import cv2
# import numpy as np
# import matplotlib.pyplot as plt
#
# def load_image(path):
#     img = cv2.imread(path, cv2.IMREAD_GRAYSCALE)
#     if img is None:
#         raise FileNotFoundError(f"Bild konnte nicht geladen werden: {path}")
#     return img
#
# def compare_masks(mask1_path, mask2_path):
#     # Masken laden
#     mask1 = load_image(mask1_path)
#     mask2 = load_image(mask2_path)
#
#     # Sicherstellen, dass die Masken die gleiche Größe haben
#     if mask1.shape != mask2.shape:
#         raise ValueError("Die Masken müssen die gleiche Größe haben")
#
#     # Absoluter Unterschied zwischen den Masken
#     diff = cv2.absdiff(mask1, mask2)
#
#     # Ausgabe
#     plt.figure(figsize=(10, 5))
#     plt.subplot(1, 3, 1)
#     plt.title("Maske 1")
#     plt.imshow(mask1, cmap='gray')
#     plt.axis('off')
#
#     plt.subplot(1, 3, 2)
#     plt.title("Maske 2")
#     plt.imshow(mask2, cmap='gray')
#     plt.axis('off')
#
#     plt.subplot(1, 3, 3)
#     plt.title("Differenz")
#     plt.imshow(diff, cmap='hot')
#     plt.axis('off')
#
#     plt.show()
#
# # Beispielaufruf
# compare_masks('../scenes/tt_francis/masks/00001.png', '../scenes/tt_francis/images_small/00001.png')

import cv2
import numpy as np
import matplotlib.pyplot as plt

def load_image(path):
    img = cv2.imread(path, cv2.IMREAD_GRAYSCALE)
    if img is None:
        raise FileNotFoundError(f"Bild konnte nicht geladen werden: {path}")
    return img

def compare_masks(mask1_path, mask2_path):
    # Masken laden
    mask1 = load_image(mask1_path)
    mask2 = load_image(mask2_path)

    # Sicherstellen, dass die Masken die gleiche Größe haben
    if mask1.shape != mask2.shape:
        raise ValueError("Die Masken müssen die gleiche Größe haben")

    # Direkte Differenz berechnen
    diff = cv2.absdiff(mask1, mask2)

    # Anzeige der Ergebnisse
    plt.figure(figsize=(10, 5))
    plt.subplot(1, 3, 1)
    plt.title("Maske 1")
    plt.imshow(mask1, cmap='gray')
    plt.axis('off')

    plt.subplot(1, 3, 2)
    plt.title("Maske 2")
    plt.imshow(mask2, cmap='gray')
    plt.axis('off')

    plt.subplot(1, 3, 3)
    plt.title("Differenz")
    plt.imshow(diff, cmap='gray')
    plt.axis('off')

    plt.show()

# Beispielaufruf
compare_masks('../scenes/tt_francis/masks/00001.png', '../scenes/tt_francis/images_small/00001.png')

import os
from PIL import Image
import numpy as np

def expand_mask(input_path, output_path):
    # Bild öffnen
    img = Image.open(input_path).convert("L")  # Graustufenbild laden

    # Bild in ein NumPy-Array umwandeln
    mask = np.array(img)

    # Maske binarisieren: 0 (weiß) und 1 (schwarz)
    binary_mask = (mask <= 127).astype(np.uint8)  # 1 für schwarz, 0 für weiß

    # Maske erweitern (schwarzen Bereich ausdehnen)
    expanded_mask = np.pad(binary_mask, pad_width=1, mode='constant', constant_values=0)
    new_mask = (
        expanded_mask[:-2, :-2] | expanded_mask[1:-1, :-2] | expanded_mask[2:, :-2] |
        expanded_mask[:-2, 1:-1] | expanded_mask[1:-1, 1:-1] | expanded_mask[2:, 1:-1] |
        expanded_mask[:-2, 2:] | expanded_mask[1:-1, 2:] | expanded_mask[2:, 2:]
    )

    # Ergebnis invertieren: zurück zu Weiß (255) und Schwarz (0)
    final_mask = (1 - new_mask) * 255

    # Ergebnis in das ursprüngliche Format zurückwandeln
    expanded_image = Image.fromarray(final_mask.astype(np.uint8))

    # Als PNG speichern
    expanded_image.save(output_path)

def process_folder(input_folder, output_folder):
    # Sicherstellen, dass der Ausgabeordner existiert
    os.makedirs(output_folder, exist_ok=True)

    # Alle Dateien im Eingabeordner durchgehen
    for filename in os.listdir(input_folder):
        if filename.lower().endswith(('.png', '.jpg', '.jpeg')):  # Nur Bilddateien verarbeiten
            input_path = os.path.join(input_folder, filename)
            output_path = os.path.join(output_folder, filename)

            print(f"Verarbeite: {input_path} -> {output_path}")
            expand_mask(input_path, output_path)

# Ordnerpfade anpassen
input_folder = "../scenes/bistro/masks"   # Ordner mit den Eingabemasken
output_folder = "../scenes/bistro/output_masks" # Ordner für die erweiterten Masken

process_folder(input_folder, output_folder)

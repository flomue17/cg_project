import numpy as np
import matplotlib.pyplot as plt
from PIL import Image

def visualize_difference(image1_path, image2_path, output_path):
    # Lade die beiden Bilder
    image1 = Image.open(image1_path)
    image2 = Image.open(image2_path)

    image1 = image1.convert('RGB')
    image2 = image2.convert('RGB')

    image1_array = np.array(image1).astype(np.float32)
    image2_array = np.array(image2).astype(np.float32)

    if image1_array.shape != image2_array.shape:
        raise ValueError("Die Bilder müssen die gleiche Größe haben!")

    difference = np.abs(image1_array - image2_array)
    difference = np.linalg.norm(difference, axis=2)/255.0

    normalized_difference = (difference - np.min(difference)) / (np.max(difference) - np.min(difference))
    normalized_difference = difference
    reversed_difference = normalized_difference

    plt.imshow(reversed_difference, cmap='turbo')
    plt.colorbar(fraction=0.025)
    plt.title("Difference Mean and Dynamic Buckets")

    # Speichern der Visualisierung
    plt.savefig(output_path, dpi=300)
    plt.show()

# Beispiel: Vergleich und Visualisierung
image1_path = './../images/mean.png'
image2_path = "./../images/buckets_dyanmic.png"
output_path = '../outputs/difference_mean_dynamic_buckets.png'

visualize_difference(image1_path, image2_path, output_path)

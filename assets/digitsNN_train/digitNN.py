import tensorflow as tf
from tensorflow.keras import layers, models
import numpy as np
import os
from glob import glob
import cv2
import shutil

# ───────────── Settings ─────────────
data_dir = "images"
saved_model_dir = "model_saved"

# ───────────── Loading data ─────────────
X = []
y = []

classes = "0123456789XY:-"  # mapping class -> character
class_to_idx = {c:i for i,c in enumerate(classes)}

for fpath in glob(os.path.join(data_dir, "*.png")):
    fname = os.path.basename(fpath)
    label_char = os.path.splitext(fname)[0]  # ex: "1" or "X" or "d"

    # special mapping for ":" and "-"
    if label_char == "d":
        label_char = ":"
    elif label_char == "m":
        label_char = "-"

    if label_char not in class_to_idx:
        continue

    img = cv2.imread(fpath, cv2.IMREAD_GRAYSCALE)
    img = cv2.resize(img, (32,32))          # ensure size 32x32
    img = img.astype(np.float32) / 255.0    # normalize 0-1
    img = np.expand_dims(img, axis=-1)      # 32x32x1
    X.append(img)
    y.append(class_to_idx[label_char])

X = np.stack(X, axis=0)
y = np.array(y)

print("Loaded images:", X.shape, "Labels:", y.shape)

# ───────────── Model ─────────────
model = models.Sequential([
    layers.Input((32,32,1)),
    layers.Conv2D(8, (3,3), activation='relu'),
    layers.MaxPooling2D((2,2)),
    layers.Conv2D(16, (3,3), activation='relu'),
    layers.MaxPooling2D((2,2)),
    layers.Flatten(),
    layers.Dense(32, activation='relu'),
    layers.Dense(len(classes), activation='softmax')
])

model.compile(optimizer='adam', loss='sparse_categorical_crossentropy', metrics=['accuracy'])
model.summary()

# ───────────── Training ─────────────
model.fit(X, y, epochs=100, batch_size=16)

# ───────────── Save model ─────────────
if os.path.exists(saved_model_dir):
    shutil.rmtree(saved_model_dir)

model.save(saved_model_dir, save_format="tf")
print(f"Model saved in folder: {saved_model_dir}")

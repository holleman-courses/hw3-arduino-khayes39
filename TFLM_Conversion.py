import tensorflow as tf
import numpy as np


model = tf.keras.models.load_model("sin_predictor.h5")

def representative_data_gen():
    """
    Yield batches of input data matching the shape and range
    that your model expects. In this case, your model expects
    sequences of length 7, e.g. (batch_size, 7).
    """
    # Suppose your model expects data roughly in [-1.0, 1.0] range
    # If you have real training data, replace this with real samples
    for _ in range(100):  # 100 batches, as an example
        # For demonstration, generate random samples from -1 to 1
        dummy_input = np.random.uniform(-1, 1, (1, 7)).astype(np.float32)
        yield [dummy_input]


converter = tf.lite.TFLiteConverter.from_keras_model(model)

# Enable optimizations
converter.optimizations = [tf.lite.Optimize.DEFAULT]

# Provide representative dataset
converter.representative_dataset = representative_data_gen

# Force full-integer (int8) quantization for input and output
converter.target_spec.supported_ops = [tf.lite.OpsSet.TFLITE_BUILTINS_INT8]
converter.inference_input_type  = tf.int8
converter.inference_output_type = tf.int8

# Convert!
tflite_model = converter.convert()

# Save the model to a file
with open("sin_predictor_int8.tflite", "wb") as f:
    f.write(tflite_model)

        
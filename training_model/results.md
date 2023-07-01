# The Results of the trained tensorflow models for motion classification

The trained models are:

- model_small
- model
- model_large

Alle models wurden einmal 600 und 1000 Epochen trainiert.

| Model | Layers | Epochs | t. loss | v. loss | m.a.e. train | m.a.e. validation |
| ----- | ------ | ------ | ------- | ------- | ------------ | ----------------- |

| model_small | (50), (15) | 600    | 0.0766  | 0.0761  | 0.1525       | 0.1520            |

## Some Text to the model results

model_small = Hat ein (50) Dense Layer und ein (15) Dense Layer. 600 Epochen. Validation loss von 0.0766 und Training loss von 0.0762. Mean absolute error: Training: 0.152 und Validation: 0.1525 -> ziemlich hoch!

Das med model war ziemlicher rotz -> vielleicht war das erste Layer zu groß -> probeire ein (50) (50) (15) Netz aus.

Das model_50_50 hat (50), (50), (15) Layer mit 600 Eopchen.
Das model_200_15_1000 hat (200), (15) mit 1000 Epochen -> Leider hat sich an der presicion nicht viel verändert, daher kann ich auch das kleinere Model verwenden.
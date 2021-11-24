import matplotlib.pyplot as plt

def draw_coco():
    """
    Draw the COCO result.
    """
    plt.figure(figsize=(10, 10))
    plt.axis('off')
    plt.imshow(plt.imread('paper/picture/coco.png'))
    plt.show()
from ultralytics import YOLO

model = YOLO("yolov8n.pt")
model.train(
    data=r"C:\Users\marak\Desktop\SofOD\pothole-18\data.yaml",
    epochs=50,
    imgsz=320,
    batch=16,
    val=True
)

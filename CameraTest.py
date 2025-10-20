import cv2
import time
import json
import websocket  # pip install websocket-client
from ultralytics import YOLO


MODEL_PATH = "../../../../../Desktop/SofOD/runs/detect/train7/weights/best.pt"
WS_URL = "ws://10.198.171.173:8080"  # Node.js server IP
SEND_INTERVAL = 0.5


model = YOLO(MODEL_PATH)


cap = cv2.VideoCapture(0)
if not cap.isOpened():
    print("❌ Could not open webcam.")
    exit()

ws = websocket.WebSocket()
try:
    ws.connect(WS_URL)
    print(f"✅ Connected to WebSocket server at {WS_URL}")
except Exception as e:
    print(f"❌ Could not connect to WebSocket server: {e}")
    cap.release()
    exit()

last_send_time = 0

while True:
    ret, frame = cap.read()
    if not ret:
        print("⚠️ Failed to grab frame")
        break

    results = model(frame, imgsz=320, verbose=False)
    num_holes = len(results[0].boxes) if results[0].boxes is not None else 0

    # Annotate frame
    annotated = results[0].plot()
    cv2.putText(annotated, f"Holes: {num_holes}", (20, 40),
                cv2.FONT_HERSHEY_SIMPLEX, 1.1, (0, 255, 0), 3)
    cv2.imshow("YOLOv8 Detection", annotated)

    # Send via WebSocket at SEND_INTERVAL
    current_time = time.time()
    if current_time - last_send_time > SEND_INTERVAL:
        try:
            ws.send(json.dumps({"holesCount": num_holes}))
        except Exception as e:
            print(f"⚠️ Failed to send WebSocket message: {e}")
        last_send_time = current_time

    if cv2.waitKey(1) & 0xFF == ord('q'):
        break

cap.release()
cv2.destroyAllWindows()
ws.close()
print("🛑 Program terminated")

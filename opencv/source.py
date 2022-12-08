import cv2
import sys
import numpy as np
from time import sleep
from statistics import median
from math import sqrt
import serial
PY3 = sys.version_info[0] == 3

if PY3:
    xrange = range

s = 0
if len(sys.argv) > 1:
    s = sys.argv[1]

win_cam = 'Camera Detection'
cv2.namedWindow(win_cam, cv2.WINDOW_NORMAL)
win_thresh = 'Threshhold View'
cv2.namedWindow(win_thresh, cv2.WINDOW_NORMAL)
win_draw = 'Object Drawing'
cv2.namedWindow(win_draw, cv2.WINDOW_NORMAL)

result = None
source = cv2.VideoCapture(s)    

ACCEPT_INPUT = 0
WRITE_COLOR = 1
WRITE_DIST = 2

state = ACCEPT_INPUT

serialPort = serial.Serial(port="COM8", baudrate=9600, bytesize=8, parity=serial.PARITY_NONE, stopbits=serial.STOPBITS_ONE)

serialString = ""

lower_blue = (105, 150, 80)
upper_blue = (130, 255, 255)

lower_red = (0, 100, 70)
upper_red = (15, 255, 255)
        
lower_green = (40, 100, 70)
upper_green = (85, 255, 255)
     
lower_yellow = (140, 100, 70)
upper_yellow = (165, 255, 255)

def angle_cos(p0,p1,p2):
    d1, d2 = (p0-p1).astype('float'), (p2-p1).astype('float')
    return abs( np.dot(d1, d2) / np.sqrt( np.dot(d1, d1)*np.dot(d2, d2) ) )

def findShapes(img):
    coords = []
    shapes = []
    for gray in cv2.split(img):
        for thrs in xrange(0,255,26):
            if thrs == 0:
                bin = cv2.Canny(gray, 0, 50, apertureSize=5)
                bin = cv2.dilate(bin, None)
            else:
                _retval, bin = cv2.threshold(gray, thrs, 255, cv2.THRESH_BINARY)
            contours, _hierarchy = cv2.findContours(bin, cv2.RETR_LIST, cv2.CHAIN_APPROX_SIMPLE)
            for cnt in contours:
                cnt_len = cv2.arcLength(cnt, True)
                cnt = cv2.approxPolyDP(cnt, 0.02*cnt_len, True)
                if len(cnt) == 4 and cv2.contourArea(cnt) > 1000:
                    x,y,w,h = cv2.boundingRect(cnt)
                    cnt = cnt.reshape(-1, 2)
                    found = False
                    for coord in coords:
                        if abs(coord[0] - x) < 50:
                            found = True
                    if not found or len(coords) == 0:
                        shapes.append(cnt)
                        coords.append((x,y,w,h))
    return shapes, coords

out_msg = ""
out_color = 'n'

while True:
    ok,frame = source.read()
    if not ok:
        continue

    x_center = frame.shape[1]/2
    y_center = frame.shape[0]/2

    frame = cv2.flip(frame,1)

    frame_draw = np.copy(frame)

    frame_hsv = cv2.cvtColor(frame, cv2.COLOR_BGR2HSV)

    blue_mask = cv2.inRange(frame_hsv, lower_blue, upper_blue)
    red_mask = cv2.inRange(frame_hsv, lower_red, upper_red)
    green_mask = cv2.inRange(frame_hsv, lower_green, upper_green)
    yellow_mask = cv2.inRange(frame_hsv, lower_yellow, upper_yellow)

    comb_mask = blue_mask | red_mask | green_mask | yellow_mask
    
    frame_mask = cv2.bitwise_and(frame, frame, mask=comb_mask)

    if state == ACCEPT_INPUT:
        try:
            if serialPort.in_waiting > 0:
                serialString = serialPort.readline()
                print("Reading line ... ", serialString.decode('Ascii'))
                if (serialString[0] - 48 == 1):
                    state = WRITE_COLOR
        
            blue_shapes, blue_coords = findShapes(blue_mask)
            red_shapes, red_coords = findShapes(red_mask)
            green_shapes, yellow_coords = findShapes(yellow_mask)
            yellow_shapes, green_coords = findShapes(green_mask)
            cv2.drawContours(frame_draw, blue_shapes, -1, (0,255,0), 2)
            cv2.drawContours(frame_draw, red_shapes, -1, (0,255,0), 2)
            cv2.drawContours(frame_draw, green_shapes, -1, (0,255,0), 2)
            cv2.drawContours(frame_draw, yellow_shapes, -1, (0,255,0), 2)

            new_min = False
            dist_min = sys.maxsize
            out_color = 'n'
            out_msg = ""
            for c in blue_coords:
                x_loc = c[0] + c[2]/2
                y_loc = c[1] + c[3]/2

                if (sqrt(round(pow(abs(x_loc - x_center),2)) + round(pow(abs(y_loc - y_center),2)))):
                    dist_min = sqrt(round(pow(abs(x_loc - x_center), 2)) + round(pow(abs(y_loc - y_center), 2)))
                    new_min = True
                    out_color = 'b'
                    out_msg = "x" + str(round(x_loc - x_center)) + " y" + str(round(y_loc - y_center))

            for c in red_coords:
                x_loc = c[0] + c[2]/2
                y_loc = c[1] + c[3]/2

                if (sqrt(round(pow(abs(x_loc - x_center),2)) + round(pow(abs(y_loc - y_center),2)))):
                    dist_min = sqrt(round(pow(abs(x_loc - x_center), 2)) + round(pow(abs(y_loc - y_center), 2)))
                    new_min = True
                    out_color = 'r'
                    out_msg = "x" + str(round(x_loc - x_center)) + " y" + str(round(y_loc - y_center))

            for c in green_coords:
                x_loc = c[0] + c[2]/2
                y_loc = c[1] + c[3]/2

                if (sqrt(round(pow(abs(x_loc - x_center),2)) + round(pow(abs(y_loc - y_center),2)))):
                    dist_min = sqrt(round(pow(abs(x_loc - x_center), 2)) + round(pow(abs(y_loc - y_center), 2)))
                    new_min = True
                    out_color = 'g'
                    out_msg = "x" + str(round(x_loc - x_center)) + " y" + str(round(y_loc - y_center))

            for c in yellow_coords:
                x_loc = c[0] + c[2]/2
                y_loc = c[1] + c[3]/2

                if (sqrt(round(pow(abs(x_loc - x_center),2)) + round(pow(abs(y_loc - y_center),2)))):
                    dist_min = sqrt(round(pow(abs(x_loc - x_center), 2)) + round(pow(abs(y_loc - y_center), 2)))
                    new_min = True
                    out_color = 'y'
                    out_msg = "x" + str(round(x_loc - x_center)) + " y" + str(round(y_loc - y_center))


        except:
            print("Exception Found")


    elif state == WRITE_COLOR:
        out_color = out_color + '\r'
        sent_bytes = serialPort.write(out_color.encode())
        print("Sent ", sent_bytes, " bytes in return message: ", out_color)
        state = WRITE_DIST
    elif state == WRITE_DIST:
        out_msg = out_msg + '\r'
        sent_bytes = serialPort.write(out_msg.encode())
        print("Sent ", sent_bytes, " bytes in return message: ", out_msg)
        state = ACCEPT_INPUT

    cv2.imshow(win_cam, frame)
    cv2.imshow(win_thresh, frame_mask)
    cv2.imshow(win_draw, frame_draw)

    key = cv2.waitKey(1)
    if key == ord('Q') or key == ord('q') or key == 27:
        break

source.release()
cv2.destroyAllWindows()


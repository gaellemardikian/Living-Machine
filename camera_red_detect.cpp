#include <opencv2/opencv.hpp>
#include <iostream>
#include <fstream>
#include <string>
#include <fcntl.h>
#include <termios.h>
#include <unistd.h>
#include <algorithm>
#include <vector>

using namespace std;
using namespace cv;

string couleur_selectionnee = "rouge";

void fichier_couleur() // lit le fichier texte pour la couleur selectionne
{
    ifstream fichier("couleur.txt");
    if (fichier.is_open())
    {
        getline(fichier, couleur_selectionnee);
        fichier.close();
    }
    else
    {
        couleur_selectionnee = "rouge";
        cout << "fichier texte de la couleur non trouvé. la couleur rouge a été sélectionnée automatiquement" << endl;
    }
}

int main()
{
    fichier_couleur();

    VideoCapture video(0); // debug port et camera
    if (video.isOpened() == 0)
    {
        cerr << "camera non accessible\n";
        return 1;
    }

    int serialPort = open("/dev/cu.usbmodem1101", O_RDWR | O_NOCTTY);
    if (serialPort < 0)
    {
        cerr << "port serie non accessible\n";
        return 1;
    }
    // lien avec arduino
    struct termios tty{};
    cfsetispeed(&tty, B9600);
    cfsetospeed(&tty, B9600);
    tty.c_cflag |= (CLOCAL | CREAD);
    tty.c_cflag &= ~CSIZE;
    tty.c_cflag |= CS8;
    tty.c_cflag &= ~PARENB;
    tty.c_cflag &= ~CSTOPB;
    tty.c_cflag &= ~CRTSCTS;
    tty.c_lflag = 0;
    tty.c_oflag = 0;
    tty.c_iflag = 0;
    tcflush(serialPort, TCIFLUSH);
    tcsetattr(serialPort, TCSANOW, &tty);

    int angle_moteur_horizontal = 90;
    int angle_moteur_vertical = 130;
    string initCmd = "90h130v\n";
    sleep(2);
    write(serialPort, initCmd.c_str(), initCmd.length());

    Mat frame, hsv, masque, masque_temp;

    while (1)
    {
        video >> frame;
        if (frame.empty())
            break;

        cvtColor(frame, hsv, COLOR_BGR2HSV);
        masque = Mat::zeros(hsv.size(), CV_8U);

        if (couleur_selectionnee == "rouge")
        {
            inRange(hsv, Scalar(0, 100, 100), Scalar(10, 255, 255), masque_temp);
            masque = masque | masque_temp;
            inRange(hsv, Scalar(160, 100, 100), Scalar(179, 255, 255), masque_temp);
            masque = masque | masque_temp;
        }
        else if (couleur_selectionnee == "vert")
        {
            inRange(hsv, Scalar(35, 40, 40), Scalar(85, 255, 255), masque_temp);
            masque = masque | masque_temp;
        }
        else if (couleur_selectionnee == "bleu")
        {
            inRange(hsv, Scalar(90, 50, 70), Scalar(130, 255, 255), masque_temp);
            masque = masque | masque_temp;
        }

        vector<vector<Point>> contours;
        findContours(masque, contours, RETR_EXTERNAL, CHAIN_APPROX_SIMPLE); // prends le contour du masque

        if (contours.size() > 0)
        {
            int i_max = 0;
            for (int i = 1; i < contours.size(); i++)
            {
                if (contourArea(contours[i]) > contourArea(contours[i_max])) // on garde la plus surface de couleur trouvé
                {
                    i_max = i;
                }
            }

            if (contourArea(contours[i_max]) > 500) // l'objet ne peut pas etre tres petit sinon on le suit pas
            {
                Rect rect = boundingRect(contours[i_max]);
                Point center(rect.x + rect.width / 2, rect.y + rect.height / 2); // essaye de mettre l'objet dans le cadre dessinné au milieu

                int cx = frame.cols / 2;
                int cy = frame.rows / 2;
                int box = 100;
                if (center.x < cx - box / 2)
                {
                    angle_moteur_horizontal = angle_moteur_horizontal + 2;
                    if (angle_moteur_horizontal > 180)
                        angle_moteur_horizontal = 180;
                }
                if (center.x > cx + box / 2)
                {
                    angle_moteur_horizontal = angle_moteur_horizontal - 2;
                    if (angle_moteur_horizontal < 0)
                        angle_moteur_horizontal = 0;
                }
                if (center.y < cy - box / 2)
                {
                    angle_moteur_vertical = angle_moteur_vertical + 2;
                    if (angle_moteur_vertical > 180)
                        angle_moteur_vertical = 180;
                }
                if (center.y > cy + box / 2)
                {
                    angle_moteur_vertical = angle_moteur_vertical - 2;
                    if (angle_moteur_vertical < 0)
                        angle_moteur_vertical = 0;
                }

                string cmd = to_string(angle_moteur_horizontal) + "h" + to_string(angle_moteur_vertical) + "v\n";
                write(serialPort, cmd.c_str(), cmd.length());

                rectangle(frame, rect, Scalar(0, 0, 255), 2);
                circle(frame, center, 5, Scalar(0, 255, 0), -1);
            }
        }

        rectangle(frame, Point(frame.cols / 2 - 50, frame.rows / 2 - 50),
                  Point(frame.cols / 2 + 50, frame.rows / 2 + 50), Scalar(255, 255, 255), 2);

        string fenetre_couleur = "Suivi de " + couleur_selectionnee;
        imshow(fenetre_couleur, frame);

        int key = waitKey(30);
        if (key == 27)
            break;
    }

    close(serialPort);
    video.release();
    destroyAllWindows();
    return 0;
}
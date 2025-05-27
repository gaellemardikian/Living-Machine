#include <opencv2/opencv.hpp>
#include <gtk/gtk.h>
#include <iostream>
#include <fstream>
#include <deque>
#include <map>
#include <fcntl.h>
#include <unistd.h>
#include <termios.h>
#include <algorithm>

using namespace cv;
using namespace std;
struct Suspect
{
    Rect face;
    bool suivre = false;
    int angle_moteur_h = 90;
    int angle_moteur_v = 140;
    int fps_apres_perte = 0;
};

// pour ne pas avoir des faux positifs, on fait une moyenne sur les 15 dernieres couleurs et les 15 derniers oui/non du port de masque

deque<string> moy_couleurs; // vecteur des 15 dernieres couleurs detectee (deque pour pouvoir faire in pop.front)
deque<string> moy_masques;  // masque oui/non
const int l_max_deque = 15;

bool maj_hist(const string &couleur, const string &masque)
{
    moy_couleurs.push_back(couleur);
    moy_masques.push_back(masque);

    if (moy_couleurs.size() > l_max_deque)
        moy_couleurs.pop_front();
    if (moy_masques.size() > l_max_deque)
        moy_masques.pop_front();
    if (moy_couleurs.size() == l_max_deque && moy_masques.size() == l_max_deque) // verifie si les 15 fps ont ete remplie
    {
        return 1;
    }
    else
    {
        return 0;
    }
}

bool history_remplie()
{ // verifie si les 15 fps ont ete remplie
    if (moy_couleurs.size() == l_max_deque && moy_masques.size() == l_max_deque)
    {
        return 1;
    }
    else
    {
        return 0;
    }
}
string couleur_f()
{
    map<string, int> freq;
    for (const auto &val : moy_couleurs)
    {
        freq[val]++;
    }
    string max_couleur_nom = "";
    int couleur_rep = 0;
    for (const auto &p : freq)
    {
        if (p.second > couleur_rep)
        {
            couleur_rep = p.second;
            max_couleur_nom = p.first;
        }
    }
    return max_couleur_nom;
}

string masque_f()
{
    map<string, int> freq;
    for (const auto &val : moy_masques)
        freq[val]++;
    string max_couleur_nom = "";
    int couleur_rep = 0;
    for (const auto &p : freq)
    {
        if (p.second > couleur_rep)
        {
            couleur_rep = p.second;
            max_couleur_nom = p.first;
        }
    }
    return max_couleur_nom;
}

void suspect_caract(const string &chemin, string &couleur_suspect, string &masque_suspect) // lit le fichier ecrit par l'interface pour les caracteristique du suspect
{
    ifstream file(chemin);
    string ligne;

    while (getline(file, ligne))
    {
        transform(ligne.begin(), ligne.end(), ligne.begin(), ::tolower);

        int pos_tshirt = ligne.find("tshirt");
        if (pos_tshirt >= 0)
        {
            int deux_points = ligne.find(":");
            couleur_suspect = ligne.substr(deux_points + 1);
        }

        int pos_masque = ligne.find("masque");
        if (pos_masque >= 0)
        {
            int deux_points = ligne.find(":");
            masque_suspect = ligne.substr(deux_points + 1);
        }
    }

    couleur_suspect.erase(remove_if(couleur_suspect.begin(), couleur_suspect.end(), ::isspace), couleur_suspect.end());
    masque_suspect.erase(remove_if(masque_suspect.begin(), masque_suspect.end(), ::isspace), masque_suspect.end());
}

string moy_couleur(const Mat &roi) // moyenne de la couleur dans le rectangle du tshirt
{
    Mat hsv;
    cvtColor(roi, hsv, COLOR_BGR2HSV); // le format bgr ne detectais pas tres bien les gris noire et blanc

    long somme_hue = 0, somme_saturation = 0, somme_lum = 0;
    int totalep = hsv.rows * hsv.cols;

    for (int y = 0; y < hsv.rows; y = y + 1)
    {
        for (int x = 0; x < hsv.cols; x = x + 1)
        {
            Vec3b pixel = hsv.at<Vec3b>(y, x);
            somme_hue = somme_hue + pixel[0];
            somme_saturation = somme_saturation + pixel[1];
            somme_lum = somme_lum + pixel[2];
        }
    }

    if (totalep == 0)
    {
        return "inconnue";
    }

    int h = somme_hue / totalep;
    int s = somme_saturation / totalep;
    int v = somme_lum / totalep;

    // cout << "H: " << h << " S: " << s << " V: " << v << endl;

    if (v < 50) // tester avec un cout du hsv et prit avec la meilleur approximation
    {
        return "noire";
    }
    else if (v > 200)
    {
        return "blanc";
    }
    else if (s < 70)
    {
        return "grise";
    }
    else if (h >= 20 && h <= 30)
    {
        return "jaune";
    }
    else if (h <= 10 || h >= 160)
    {
        return "rouge";
    }
    else if (h >= 35 && h <= 85)
    {
        return "vert";
    }
    else if (h >= 90 && h <= 130)
    {
        return "bleue";
    }

    return "inconnue";
}

string masque_detect(Mat &faceROI, CascadeClassifier &mouthCascade, CascadeClassifier &eyesCascade) // detecter s'il y'a un masque
{
    Mat gray;
    cvtColor(faceROI, gray, COLOR_BGR2GRAY);
    equalizeHist(gray, gray);
    vector<Rect> mouths, eyes;
    Rect rectBouche(0, (2 * gray.rows) / 3, gray.cols, gray.rows / 3);                // definit une zone ou peut se situer la bouche
    mouthCascade.detectMultiScale(gray(rectBouche), mouths, 1.1, 3, 0, Size(20, 20)); // detection ou non la bouche dans le rectangle de la bouche definit
    eyesCascade.detectMultiScale(gray, eyes, 1.1, 3, 0, Size(15, 15));                // detection des yeux ou non dans le visage
    if (mouths.empty() && !eyes.empty())                                              // il detecte des yeux mais pas une bouche -> masque, essqie avec que la detection ou non de la bouche c'étais moins performant car si on tourner la tete il pensait qu'on portait un masque
    {
        return "oui";
    }
    else
    {
        return "non";
    }
}

int main()
{
    gtk_init(0, nullptr);
    string couleur_suspect, masque_suspect;
    suspect_caract("config.txt", couleur_suspect, masque_suspect);

    CascadeClassifier faceCascade, mouthCascade, eyesCascade;
    if (faceCascade.load("haarcascade_frontalface_default.xml") == 0 || mouthCascade.load("haarcascade_mcs_mouth.xml") == 0 || eyesCascade.load("haarcascade_eye.xml") == 0)
    {
        cerr << "fichier xml non trouvé" << endl;
        exit(1);
    }

    VideoCapture cap(0);
    if (cap.isOpened() == 0) // camera non trouvé
        exit(1);

    int Port = open("/dev/cu.usbmodem1101", O_RDWR | O_NOCTTY);
    if (Port < 0) // port non trouvé
    {
        cerr << "port série non trouvé" << endl;
        exit(1);
    }
    sleep(2); // sans ca le moteur ne bouger pas a la position initale

    struct termios tty{};
    tcgetattr(Port, &tty);
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
    tcflush(Port, TCIFLUSH);
    tcsetattr(Port, TCSANOW, &tty);
    Suspect suspect;
    string init_cmd = to_string(suspect.angle_moteur_h) + "h" + to_string(suspect.angle_moteur_v) + "v\n";
    write(Port, init_cmd.c_str(), init_cmd.length());

    KalmanFilter kalman(4, 2, 0); // v cst
    Mat state(4, 1, CV_32F);
    Mat meas(2, 1, CV_32F);
    kalman.transitionMatrix = (Mat_<float>(4, 4) << 1, 0, 1, 0, 0, 1, 0, 1, 0, 0, 1, 0, 0, 0, 0, 1);
    setIdentity(kalman.measurementMatrix);
    setIdentity(kalman.processNoiseCov, Scalar::all(0.0001));
    setIdentity(kalman.measurementNoiseCov, Scalar::all(0.1));
    setIdentity(kalman.errorCovPost, Scalar::all(0.1));

    Mat frame;
    while (1)
    {
        cap >> frame;
        if (frame.empty())
            break;
        vector<Rect> faces;
        faceCascade.detectMultiScale(frame, faces);
        Mat pred = kalman.predict();
        int predX = pred.at<float>(0);
        int predY = pred.at<float>(1);                                  // prediction de la position x y du visage
        circle(frame, Point(predX, predY), 5, Scalar(0, 255, 255), -1); // point jaune

        bool detect_visage = false;
        for (size_t i = 0; i < faces.size(); ++i)
        {
            Rect face = faces[i];

            Point center(face.x + face.width / 2, face.y + face.height / 2);

            if (suspect.suivre && (face & suspect.face).area() > 0) // pour ne pas qu'il change des qu'il voit un autre visage
            {
                detect_visage = true;
                meas.at<float>(0) = center.x;
                meas.at<float>(1) = center.y;
                kalman.correct(meas); // si le visage est detectee on corrige

                int cX = frame.cols / 2, cY = frame.rows / 2, box = 100; // petit carre au centre de l'image
                if (center.x < cX - box / 2)
                {
                    suspect.angle_moteur_h = suspect.angle_moteur_h + 2;
                }
                else if (center.x > cX + box / 2)
                {
                    suspect.angle_moteur_h = suspect.angle_moteur_h - 2;
                }
                if (center.y < cY - box / 2)
                {
                    suspect.angle_moteur_v = suspect.angle_moteur_v + 2;
                }
                else if (center.y > cY + box / 2)
                {
                    suspect.angle_moteur_v = suspect.angle_moteur_v - 2;
                } // ajustement pour que le visage soit dans le petit carree

                string cmd = to_string(suspect.angle_moteur_h) + "h" + to_string(suspect.angle_moteur_v) + "v\n";
                write(Port, cmd.c_str(), cmd.length());

                rectangle(frame, Point(cX - 50, cY - 50), Point(cX + 50, cY + 50), Scalar(255, 255, 255), 2);
                putText(frame, "suspect retrouve", Point(30, 30), FONT_HERSHEY_SIMPLEX, 1.0, Scalar(0, 0, 255), 2);
                rectangle(frame, face, Scalar(0, 0, 255), 2);
                suspect.face = face;
                break;
            }

            Rect tshirtROI(face.x, face.y + face.height + face.height / 2, face.width, face.height / 2);
            if (tshirtROI.y + tshirtROI.height > frame.rows)
            {
                continue;
            }
            string couleur = moy_couleur(frame(tshirtROI));
            Mat faceROI = frame(face);
            string masque = masque_detect(faceROI, mouthCascade, eyesCascade);
            maj_hist(couleur, masque);

            putText(frame, "tshirt: " + couleur, Point(face.x, face.y + face.height + 20), FONT_HERSHEY_SIMPLEX, 0.6, Scalar(0, 255, 0), 2);
            putText(frame, "masque: " + masque, Point(face.x, face.y - 10), FONT_HERSHEY_SIMPLEX, 0.6, Scalar(0, 0, 255), 2);
            rectangle(frame, face, Scalar(255, 0, 0), 2);

            if ((maj_hist(couleur, masque)) && couleur_f() == couleur_suspect && masque_f() == masque_suspect)
            {
                suspect.face = face;
                suspect.suivre = true;
                meas.at<float>(0) = center.x;
                meas.at<float>(1) = center.y;
                kalman.correct(meas);
            }
        }

        if (!detect_visage && suspect.suivre)
        {
            suspect.fps_apres_perte++;

            if (suspect.fps_apres_perte <= 10) // suit par rapport a la prediction de kalman ne suit pas a l'infini la prediction quand il trouve pas le visage sinon il se bloquait toujours en 180 ou 0
            {
                int cX = frame.cols / 2, cY = frame.rows / 2, box = 100;
                if (predX < cX - box / 2)
                {
                    suspect.angle_moteur_h = suspect.angle_moteur_h + 2;
                }
                else if (predX > cX + box / 2)
                {
                    suspect.angle_moteur_h = suspect.angle_moteur_h - 2;
                }
                if (predY < cY - box / 2)
                {
                    suspect.angle_moteur_v = suspect.angle_moteur_v + 2;
                    ;
                }
                else if (predY > cY + box / 2)
                {
                    suspect.angle_moteur_v = suspect.angle_moteur_v - 2;
                }

                string cmd = to_string(suspect.angle_moteur_h) + "h" + to_string(suspect.angle_moteur_v) + "v\n";
                write(Port, cmd.c_str(), cmd.length());

                rectangle(frame, Point(cX - 50, cY - 50), Point(cX + 50, cY + 50), Scalar(255, 255, 255), 2);
                putText(frame, "recherche du suspect", Point(30, 30), FONT_HERSHEY_SIMPLEX, 1.0, Scalar(0, 255, 255), 2);
            }
            else
            {
                suspect.suivre = false;
                suspect.fps_apres_perte = 0;
            }
        }
        else if (detect_visage && suspect.suivre)
        {
            suspect.fps_apres_perte = 0;
        }
        imshow("suivi du suspect", frame); // esc pour sortir
        if (waitKey(30) == 27)
        {
            break;
        }
    }

    close(Port);
    cap.release();
    destroyAllWindows();
    return 0;
}

# gr2_03


# Robot Police - Living Machine : Projet C++
Notre nom fait référence à notre projet libre. Notre système simule le comportement d'un policier en tentant de détecter et suivre un "suspect" à partir de critères prédéfinis.

## Sommaire 

1. [Description](#description)
2. [Prérequis](#prerequis)
3. [Installation](#installation)
4. [Usage](#usage)
5. [Auteurs et Remerciements](#auteurs-et-remerciements)
6. [Licence](#licence)


## Description

L’objectif est d'implémenter un programme pilotant une machine vivante, inspirée du style d’animation de Pixar. Pour cela, on dispose d’une caméra Pan-Tilt alimentée par 2 servomoteurs, que l'on peut contrôler grâce à une carte Arduino UNO. 
 
L'idée est donc que notre système réagisse à des interactions visuelles de manière dynamique, donnant alors l'impression que la machine est "vivante".

Le projet est divisé en deux parties :

* **Figure imposée** : La caméra détecte une couleur qu'elle doit suivre en temps réel, ici rouge, vert ou bleu à l’aide d’OpenCV notamment.

* **Figure libre** : Un Robot Police qui recherche un criminel à partir de deux critères définis dans l’interface : la couleur de son tee-shirt et s'il porte un masque. L’utilisateur sélectionne les paramètres qu'il souhaite, qui sont alors enregistrés dans un fichier, puis le programme lance un système de reconnaissance en se basant sur ces paramètres.


## Prérequis

### Matériel :

* Carte Arduino UNO
* Webcam montée
* 2 servomoteurs 

### Logiciels à installer :

Avant de compiler ou d'exécuter le programme, installez : 

* OpenCV pour le traitement d'image : Dans le terminal, tapez la commande :
(sur Mac avec Homebrew)

```bash
brew install opencv
``` 

* GTK : permet de créer l'interface graphique (nécessaire d'être sous Linux). Dans le terminal :

```bash
sudo apt install libgtk-3-dev
``` 

* g++ : permet de compiler en C++, tapez dans le terminal les commandes : 

```bash
sudo apt update
sudo apt install g++
```
On pourra vérifier la version avec :

```bash
g++ --version
```

* Arduino IDE : afin de téléverser le code sur la carte Arduino UNO, installez via le lien : 
    
    https://downloads.arduino.cc/arduino-ide/arduino-ide_2.3.6_Windows_64bit.exe

Note : Dans notre projet, le port série utilisé par défaut dans le code (car configuration Mac) est :

```bash
/dev/cu.usbmodem1101
```
Dans le cas d'une exécution sur Linux ou Windows, le nom du port série sera différent. Il est donc nécessaire de vérifier le port série auquel l'Arduino est connecté et d'adapter ce chemin dans le code source avant d’exécuter le programme, sinon la communication avec l’Arduino ne pourra pas s’établir correctement.

Il convient également de préciser que lors de l'exécution du code Arduino, il est nécessaire de fermer l'IDE Arduino sinon, le programme ne fonctionnera pas.

## Installation

Pour compiler les différents programmes, on s'assurera d'abord d'être dans le dossier où se trouvent les fichiers sources. On pourra alors taper dans le terminal les commandes suivantes : 

#### Compilation du suivi de couleur
```bash
g++ camera_red_detect.cpp -o camera_red_detect `pkg-config --cflags --libs opencv4`
```

#### Compilation de l'interface graphique
```bash
g++ robot_police_cam_mac.cpp -o robot_police_cam_mac `pkg-config --cflags --libs opencv4`
```
#### Compilation du Robot Police
```bash
g++ interface.cpp -o interface `pkg-config --cflags --libs gtk+-3.0`
```

## Usage


Les fichiers HaarCascade en .xml sont nécessaires pour exécuter les programmes utilisant la détection d’images.
Ainsi, avant de lancer le programme, placez dans le même dossier que l'exécutable les fichiers suivants :

* haarcascade_frontalface_default.xml
* haarcascade_mcs_mouth.xml
* haarcascade_eye_tree_eyeglasses.xml

https://github.com/opencv/opencv/tree/3.4/data/haarcascades

Lorsque le programme est compilé, il peut être exécuté avec la commande suivante :

#### Interface graphique

```bash
./interface
```

On pourra alors :

* Choisir un mode : Suivi Couleur ou Robot Police
    * Dans le premier cas, on sélectionne alors la couleur désirée entre rouge, bleu et vert puis le mode Suivi de Couleur crée un fichier `couleur.txt` avec la couleur choisie et lance `camera_red_detect`. 
    * Dans le deuxième cas, on sélectionne la couleur du suspect puis s'il porte un masque ou non. Le mode Robot Police crée alors un fichier `config.txt` avec les paramètres choisis, puis lance `robot_police_cam_mac`.

* Cliquer sur "Lancer" pour démarrer le programme correspondant.



## Auteurs et Remerciements

- Mardikian Gaëlle - Projet imposé / Projet libre
- Saandi Diyana - Interface graphique / Projet libre

Ce projet a été réalisé sous la supervision de Ludovic Saint-Bauzel et Ousmane Ndiaye - ROB3 -  Polytech Sorbonne.
## Licence

Ce projet a été réalisé dans le cadre du module Projet en langage C en ROB3 à Polytech Sorbonne.
READ ME

Tutorat système

Rémi Guillomon et Guillaume Cremaschi

Objectif : Utiliser un shield Arduino de jeu et assurer la liaison USB à un PC.

I. Compilations et exécution

Les compilations et l'exécution se déroulent en plusieurs étapes.

-  Étape 1: Vérifiez que la carte ne soit pas flashée.

```
lsusb
```


Si l'ATMega16u2 est déjà flashé, il faut enregistrer le binaire du lien : https://raw.githubusercontent.com/arduino/ArduinoCore-avr/master/firmwares/atmegaxxu2/arduino-usbserial/Arduino-usbserial-uno.hex 

Court-circuitez les broches GND et RESET de l'ATMega16U2 sur l'ICSP de ce micro-contrôleur

Puis écrivez dans un terminal :

```
\> dfu-programmer atmega16u2 erase
\> dfu-programmer atmega16u2 flash Arduino-usbserial-uno.hex
\> dfu-programmer atmega16u2 reset
```

Débranchez et rebranchez le port USB de la carte.

-  Étape 2 : Compilation et chargement du programme main.c dans l'ATMega328P

Dans un terminal, entrez ces commandes.
```
make all
make upload
```
Passez en root pour l'upload si nécessaire.

Vous pouvez tester ce progrmme directement avec la commande

```
minicom -8 -o -b 9600 -D /dev/ttyACM0
```


-  Étape 3: Flasher l'ATMega16U2 avec le programme de la partie 3

Entrez dans le répertoire suivant et compilez
```
cd lufa-master/PolytechLille/PAD/
make all
```

Puis Flashez le micro-contrôleur :
Court-circuitez les broches GND et RESET de l'ATMega16U2 sur l'ICSP de ce micro-contrôleur et tapez ces lignes

```
dfu-programmer atmega16u2 erase
dfu-programmer atmega16u2 flash PAD.hex
dfu-programmer atmega16u2 reset
```

Débranchez et rebranchez le port USB de la carte à nouveau.

-  Étape 4: Compilation du programme sur le PC

Placez-vous dans le répertoire Pgm PC

```
cd ../../../Pgm\ PC/
```

et compilez.

```
gcc -o exe init_USB.c -lusb-1.0 -Wall -Wextra
```

-  Etape 5 :

Vous pouvez lancer le programme en exécutant le fichier généré.
```
./exe
```

II. Etat du programme

-  Ce qui fonctionne :

	Le programme de l'ATMega328P est fonctionnel. Lors d'un changement d'état des boutons ou d'un déplacement du joystick, il envoie à la suite un octet de début de trame, l'octet associé au boutons, la valeur de l'axe x du joystick et celle de l'axe y. Ces trois derniers octets sont mis en forme comme conseillé dans l'énoncé.

	La réclamation des interfaces se déroule sans accroc.

	La connexion entre l'ATMega16U2 et le PC par les points d'accès s'établit correctement. De plus, l'envoi des données sur la liaison série par l'ATMega328P fonctionne (testé avec minicom).


-  Ce qui ne fonctionne pas :

	Nous n'avons pas eu l'occasion de réaliser la commande des LED depuis l'exécutable du PC.

	La réception des données sur la liaison série de l'ATmega16U2, via la fonction Reception_Serie() n'est en effet pas opérationnelle. Une fois initialisés, les octets des boutons et des axes des joysticks, sont bien transmis au PC (et affichés sur le terminal) mais leur valeur ne change pas malgré les intéractions avec la manette.

	Il est possible que la commande des LED via le PC ne soit pas non plus fonctionnelle à cause du premier point abordé.









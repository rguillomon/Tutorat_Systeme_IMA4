READ ME

Tutorat système

Rémy Guillomon // Guillaume Cremaschi

Objectif : Créer une manette avec une carte Arduino relier en connexion USB avec un PC.

Pour compiler, lancer et ultiliser les programmes :

Étape 1: Vérifier que la carte ne soit pas flashée.
>lsusb
On doit voir Arduino apparaitre.

Si flashée : enregistrer le binaire de l'énoncé. Écrire les lignes de codes de l'énoncé.

Étape 2 :
Compiler et Upload le programme main.c sur l'arduino.
>gcc -o main main.c -Wall  (pas sur que ce soit nécessaire)
>make all
>make upload

Le programme est maintenant sur l'Arduino.

Étape 3:
Flasher la carte
>>cd lufa-master/
>>cd PolytechLille PAD/
>>make all
>>dfu-programmer atmega16u2 erase
>>dfu-programmer atmega16u2 flash PAD.hex
>>dfu-programmer atmega16u2 reset

Étape 4:
Que faut-il faire pour implanter la partie 1 ?



Ce qui marche :
_ La Partie 2, fonctionne correctement, compile sans warnings.
_ La Partie 1, compile sans warning.

Ce qui ne marche pas :     (La partie 3 marche-t-elle ?)
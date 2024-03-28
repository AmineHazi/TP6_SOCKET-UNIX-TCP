# TP N°6 Socket UNIX connecté

Ce petit programme est une application légère de communication en C utilisant des sockets UNIX pour établir une communication bidirectionnelle entre un serveur et un client. Le serveur traite chaque client dans un thread distinct, permettant une gestion efficace de multiples clients simultanément, et sauvegarde les interactions dans des logs.

Dans cette version tout les clients partagent le meme numéro à deviner, et quand un des clients le trouve le reste sont notiifé cette approche nous demande de garder le descripteur de chaque client et de rajouter un thread qui écoute le server dans le programme client.


## Fonctionnement 

Le programme genere un nombre aléatoir pour tout les client qui se connect, lorsqu'un client se connect au server pn sauvegrade sa socket de service dans une liste partagé par tout les thread, lorsqu'un client trouve le nombre aléatoir on notifie les autres client connecté qu'un des client à trouver le nombre en utilisant la liste des socket de service qu'on a sauvegarder, lorsque le client ressoie le message sous la forme d'un token "CLIENT_FOUND" il ferme la communication et debloque le thread qui le gére.

## Difficulté rencontré 

Utiliser cette approche pour gerer tout les client à la fois fait qu'un client peux recevoir un message du serveur sans envoyer aucun message, et en sacant que le "send" du client est bloquant on se retrouve donc obligé de rajouter un thread d'écoute de server dans le client.


## Commencer

Ces instructions vous aideront à mettre en place une copie du projet en cours d'exécution sur votre système local à des fins de développement et de test.

## Prérequis

Pour compiler et exécuter ce projet, vous aurez besoin de :

- GCC (GNU Compiler Collection)
- GNU Make

### Exécution

Pour exécuter le programme, il faut le compiler puis lancer le serveur et le client avec les commandes suivantes :

```bash
make
./bin/server
./bin/client
```

### Supprimer les logs et les fichiers temporaires

Pour supprimer les logs et les fichiers temporaires, il y a les règles clean et cleanlogs dans le `Makefile`. Pour les utiliser, on utilise les commandes suivantes :

```bash
make clean
make cleanlogs
```

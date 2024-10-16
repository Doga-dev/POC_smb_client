
# POC smb client

## Contexte

Ce projet est une Preuve de Concept pour créer une interface à la bibliothèque libsmbclient qui réponde au exigences suivantes :

1. Permettre un accès à plusieurs dossiers partagés sur plusieurs serveurs
2. Permettre un accès via de multiple clients, potentiellement dans des threads différents
3. Détecter la perte de connexion avec un dossier partagé
4. Permettre l'abandon de l'opération en cours suite à la perte de connexion
5. Permettre la reprise de la dernière opération interrompu par la perte de connexion après la récupération de la liaison

## Etat actuel

Les 3 premières éxigences sont remplies. 

Les 2 derniers points ne le sont pas car, actuellement, la détection de la perte de connexion entraine obligatoirement l'abandon de l'opération en cours.
Cet abandon ne doit pas être bien réalisé car, après la récupération de la connexion, l'exécution d'une nouvelle opération entraine fréquemment le crash de l'application.



# Fonctions de soutien réseau de l'onduleur & limite de puissance dynamique — Fiche d'information

Un aperçu en langage clair des fonctions de soutien réseau d'un micro-onduleur PV moderne : à quoi
sert chacune, ce que l'on observe et comment elles se combinent. Sans détails techniques internes —
seulement ce que les fonctions font et signifient.

## En une image

Chaque fonction agit sur l'une des deux grandeurs que l'onduleur injecte sur le réseau :

- **Puissance active (Watt)** — la quantité d'énergie réellement injectée.
- **Puissance réactive / facteur de puissance** — le soutien de la tension et de la qualité réseau.

Plus une fonction purement de **sécurité** qui ne fait que déconnecter.

Deux règles simples expliquent leur combinaison :

1. **Les fonctions de puissance active se cumulent comme des limites — la plus stricte l'emporte.**
   Plusieurs fonctions peuvent plafonner ou réduire la sortie en même temps ; l'onduleur suit
   toujours la limite la plus basse en vigueur.
2. **Les fonctions de puissance réactive sont des alternatives — une seule est active à la fois.**
   Il existe quatre façons de déterminer la puissance réactive ; un profil réseau en utilise
   exactement une.

---

## Puissance active — « combien est injecté »

**DPL — Limite de puissance dynamique**
- Ce que c'est : un plafond de sortie réglable en temps réel, imposé de l'extérieur (p. ex. par une
  passerelle / un gestionnaire d'énergie domestique pour l'injection zéro ou la gestion de
  l'injection).
- Ce que l'on observe : la sortie suit la limite commandée en douceur, appliquée par petits pas et
  seulement après stabilisation de la nouvelle consigne, donc sans à-coups ni scintillement. Elle
  conserve un petit minimum garanti (environ 2 % de la puissance nominale) et répartit la limite sur
  les entrées de l'onduleur. La DPL seule ne coupe jamais totalement l'onduleur — un véritable arrêt
  passe par une commande d'arrêt distincte.
- Selon les versions de firmware et les modèles (observé) :
    - HMS-4T V01.00.27 — limite de base : plafonnement + rampe douce uniquement.
    - HMS-4T V01.01.12 — ajoute une gestion connexion/déconnexion par entrée (par MPPT)(PDL).
    - HMS-4T V02.00.04 — ajoute le minimum garanti (~2 %) et applique les changements de limite plus
      en douceur (regroupé en un seul régulateur).
    - HMS-2T (V01.00.08 à V01.03.09) — limite de base uniquement (plafonnement + rampe) ; pas de
      gestion par entrée ni de minimum garanti, soit encore au niveau du HMS-4T V01.00.27.

**APC — Contrôle de la puissance active**
- Ce que c'est : le cadre qui active le contrôle de la puissance active et fixe la vitesse à laquelle
  la sortie peut varier (démarrage progressif et vitesse de rampe).
- Ce que l'on observe : les variations de puissance sont douces et conformes aux normes, au lieu
  d'être brutales.

**FW — Fréquence-Watt**
- Ce que c'est : réduction automatique de la sortie quand la fréquence réseau monte trop haut, et
  reprise quand elle se stabilise — une réponse obligatoire de stabilisation du réseau.
- Ce que l'on observe : bridage temporaire lors des surfréquences ; entièrement automatique.

**VW — Tension-Watt**
- Ce que c'est : le même principe piloté par la tension réseau — réduire la sortie quand la tension
  monte trop haut.
- Ce que l'on observe : bridage temporaire quand la tension locale est élevée ; entièrement
  automatique.

## Puissance réactive / qualité — « soutien de tension & facteur de puissance » (en choisir une)

**SPF — Facteur de puissance spécifié**
- Ce que c'est : l'onduleur maintient un facteur de puissance fixe (un rapport fixe entre réactif et
  actif).
- Usage : la façon la plus simple de respecter un facteur de puissance imposé.

**WPF — Facteur de puissance selon la puissance**
- Ce que c'est : le facteur de puissance varie selon le niveau de sortie — neutre à faible puissance,
  plus soutenant proche de la pleine puissance.
- Usage : soutien de tension qui n'intervient qu'en forte production.

**Volt-Var (aussi appelé « CC »)**
- Ce que c'est : la puissance réactive suit la tension réseau — absorber quand la tension est haute,
  fournir quand elle est basse — pour maintenir activement la tension locale stable.
- Usage : le mode de soutien de tension le plus actif, là où le gestionnaire de réseau l'exige.

**RPC — Contrôle de la puissance réactive**
- Ce que c'est : une consigne de puissance réactive fixe sur commande, indépendante de la sortie ou
  de la tension.
- Usage : consigne réactive directe par le gestionnaire de réseau.

## Sécurité

**ID — Détection d'îlotage (anti-îlotage)**
- Ce que c'est : détecte la perte du réseau public et déconnecte, afin que l'onduleur n'alimente
  jamais un réseau mort.
- Ce que l'on observe : rien en fonctionnement normal ; en cas de perte du réseau, l'onduleur se
  coupe et reste coupé jusqu'au retour du réseau. Cela prime toujours sur tout le reste.
- Marche/arrêt : **activée**, elle ne se contente pas de surveiller les seuils — elle recherche
  activement une déviation de fréquence anormale persistante et déconnecte si elle dure ;
  **désactivée**, seules les fenêtres standard de sur/sous-tension et sur/sous-fréquence protègent
  contre une perte de réseau.

---

## Comment elles fonctionnent ensemble — en un coup d'œil

| Fonction | Agit sur | Groupe | Coexiste avec | Exclut |
|---|---|---|---|---|
| DPL | limite de sortie (live) | puissance active | APC, FW, VW | — |
| APC | vitesse / activation | puissance active | DPL, FW, VW | — |
| FW  | sortie selon fréquence | puissance active | DPL, APC, VW | — |
| VW  | sortie selon tension | puissance active | DPL, APC, FW | — |
| SPF | facteur de puissance fixe | puissance réactive | une fonction de puissance active | WPF, Volt-Var, RPC |
| WPF | facteur de puissance selon la sortie | puissance réactive | une fonction de puissance active | SPF, Volt-Var, RPC |
| Volt-Var / CC | réactif selon tension | puissance réactive | une fonction de puissance active | SPF, WPF, RPC |
| RPC | puissance réactive fixe | puissance réactive | une fonction de puissance active | SPF, WPF, Volt-Var |
| ID  | déconnexion sur perte réseau | sécurité | tout | — (prime sur tout au déclenchement) |

Remarques :
- Les fonctions de puissance active ne se contredisent jamais — elles se recoupent, et la limite la
  plus stricte s'applique.
- Les quatre fonctions de puissance réactive sont mutuellement exclusives. Un profil réseau en active
  normalement exactement une ; si plusieurs étaient activées à la fois, l'onduleur n'en applique
  qu'**UNE**, selon une priorité fixe — **Volt-Var, puis Facteur de puissance selon la puissance
  (WPF), puis Facteur de puissance spécifié (SPF), puis Contrôle de la puissance réactive (RPC), puis
  un cinquième mode réactif basé sur une courbe** — les autres sont ignorées. Elles ne s'additionnent
  jamais.
- La puissance active et la puissance réactive partagent la capacité totale de l'onduleur ; à très
  forte sortie, le soutien réactif disponible est donc réduit (et inversement).
- La détection d'îlotage est une protection indépendante ; lorsqu'elle déconnecte, toute la sortie
  s'arrête.

## Bon à savoir (comportement réel de l'onduleur)

- **Chaque fonction se configure individuellement** dans le profil réseau — sa propre activation plus
  ses propres seuils ou sa courbe. Un profil utilise donc n'importe quelle combinaison de fonctions
  de puissance active avec exactement un mode de puissance réactive.
- **La limite de puissance active la plus stricte l'emporte toujours.** Si, par exemple, une limite
  d'injection (DPL) et une réduction sur surfréquence (FW) s'appliquent en même temps, l'onduleur
  suit celle qui autorise le moins — automatiquement, sans conflit entre elles.
- **Les limites sont appliquées en douceur, non brutalement** — petits pas avec un bref temps de
  stabilisation — c'est pourquoi la sortie monte en rampe au lieu de sauter à l'arrivée d'une
  nouvelle limite.
- **Le soutien réactif cède la place à la puissance active près de la pleine sortie**, car les deux
  puisent dans la même capacité totale ; davantage de marge pour le soutien de
  tension/facteur de puissance apparaît dès que la puissance active diminue.

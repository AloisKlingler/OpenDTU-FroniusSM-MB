# Netzstützungs-Funktionen & Dynamische Leistungsbegrenzung — Infoblatt

Eine allgemein verständliche Übersicht der Netzstützungs-Funktionen eines modernen
PV-Mikro-Wechselrichters: wofür jede Funktion da ist, was man merkt und wie sie zusammenwirken.
Ohne technische Interna — nur, was die Funktionen tun und bedeuten.

## In einem Bild

Jede Funktion steuert eine von zwei Größen, die der Wechselrichter ins Netz gibt:

- **Wirkleistung (Watt)** — wie viel Energie tatsächlich eingespeist wird.
- **Blindleistung / Leistungsfaktor** — wie das Gerät Netzspannung und Netzqualität stützt.

Dazu eine reine **Schutz**-Funktion, die nur trennt.

Zwei einfache Regeln erklären das Zusammenspiel:

1. **Die Wirkleistungs-Funktionen wirken als Grenzen zusammen — die strengste gewinnt.** Mehrere
   Funktionen können die Leistung gleichzeitig begrenzen; der Wechselrichter folgt stets der
   niedrigsten aktuell gültigen Grenze.
2. **Die Blindleistungs-Funktionen sind Alternativen — nur eine ist gleichzeitig aktiv.** Es gibt
   vier verschiedene Wege, die Blindleistung zu bestimmen; ein Netzprofil nutzt genau einen davon.

---

## Wirkleistung — „wie viel eingespeist wird"

**DPL — Dynamische Leistungsbegrenzung**
- Was es ist: eine laufend verstellbare Obergrenze der Ausgangsleistung, von außen vorgegeben
  (z. B. durch ein Gateway/Energiemanagement für Nulleinspeisung oder Einspeisesteuerung).
- Was man merkt: die Leistung folgt der Vorgabe sanft, in kleinen Schritten und erst, wenn das
  neue Ziel kurz stabil ist — sie springt oder flackert also nicht. Es bleibt ein kleines
  garantiertes Minimum (rund 2% der Nennleistung), und die Grenze wird auf die Eingänge des
  Wechselrichters verteilt. DPL allein schaltet den Wechselrichter nie ganz ab — eine echte
  Abschaltung erfolgt über einen separaten Aus-Befehl.
- Über Firmware-Versionen und Modelle (beobachtet):
    - HMS-4T V01.00.27 — einfache Begrenzung: nur Kappen + weiche Rampe.
    - HMS-4T V01.01.12 — kommt mit Zu-/Abschaltung pro Eingang (pro MPPT) dazu (PDL).
    - HMS-4T V02.00.04 — zusätzlich das garantierte ~2%-Minimum und weichere Anwendung der
      Grenze (in einen Regler zusammengefasst).
    - HMS-2T (V01.00.08 bis V01.03.09) — nur einfache Begrenzung (Kappen + Rampe); keine
      Pro-Eingang-Abschaltung und kein garantiertes Minimum, also noch auf dem Stand von
      HMS-4T V01.00.27.

**APC — Wirkleistungssteuerung (Active Power Control)**
- Was es ist: der Rahmen, der die Wirkleistungssteuerung aktiviert und festlegt, wie schnell sich
  die Leistung ändern darf (Sanftanlauf und Rampenrate).
- Was man merkt: Leistungsänderungen laufen weich und normkonform statt abrupt.

**FW — Frequency-Watt (Frequenz-Wirkleistung)**
- Was es ist: automatische Absenkung der Leistung, wenn die Netzfrequenz zu hoch steigt, und
  Rückkehr, wenn sie sich beruhigt — eine verpflichtende Netzstützung.
- Was man merkt: zeitweises Abregeln bei Überfrequenz; vollautomatisch.

**VW — Volt-Watt (Spannungs-Wirkleistung)**
- Was es ist: dasselbe Prinzip, gesteuert von der Netzspannung — Leistung absenken bei zu hoher
  Spannung.
- Was man merkt: zeitweises Abregeln bei hoher lokaler Spannung; vollautomatisch.

## Blindleistung / Netzqualität — „Spannungs- & Leistungsfaktor-Stützung" (eine wählen)

**SPF — Fester Leistungsfaktor (Specified Power Factor)**
- Was es ist: der Wechselrichter hält einen festen Leistungsfaktor (festes Verhältnis von Blind-
  zu Wirkleistung).
- Einsatz: einfachster Weg, einen geforderten Leistungsfaktor einzuhalten.

**WPF — Watt-Power-Factor (Leistungsfaktor über Leistung)**
- Was es ist: der Leistungsfaktor ändert sich mit der Ausgangsleistung — neutral bei geringer,
  stützender nahe Volllast.
- Einsatz: Spannungsstützung, die nur bei hoher Erzeugung greift.

**Volt-Var (auch „CC" genannt)**
- Was es ist: die Blindleistung folgt der Netzspannung — aufnehmen bei hoher, liefern bei
  niedriger Spannung — um die lokale Spannung aktiv stabil zu halten.
- Einsatz: der aktivste Spannungsstütz-Modus, wo der Netzbetreiber ihn verlangt.

**RPC — Blindleistungssteuerung (Reactive Power Control)**
- Was es ist: ein fester Blindleistungs-Sollwert auf Befehl, unabhängig von Leistung oder
  Spannung.
- Einsatz: direkte Blindleistungs-Vorgabe durch den Netzbetreiber.

## Schutz

**ID — Inselnetz-Erkennung (Anti-Islanding)**
- Was es ist: erkennt den Wegfall des Versorgungsnetzes und trennt, damit der Wechselrichter nie
  ein totes Netz speist.
- Was man merkt: im Normalbetrieb nichts; bei Netzausfall schaltet das Gerät ab und bleibt aus,
  bis das Netz zurück ist. Das hat immer Vorrang vor allem anderen.
- An/Aus: **eingeschaltet** überwacht sie nicht nur die Grenzwerte, sondern sucht aktiv nach einer
  anhaltenden abnormalen Frequenzabweichung und trennt, wenn sie andauert; **ausgeschaltet**
  schützen nur die normalen Über-/Unterspannungs- und -frequenzfenster gegen ein fehlendes Netz.

---

## Zusammenspiel — auf einen Blick

| Funktion | Steuert | Gruppe | Läuft neben | Schließt aus |
|---|---|---|---|---|
| DPL | Leistungsgrenze (live) | Wirkleistung | APC, FW, VW | — |
| APC | Änderungsrate / Freigabe | Wirkleistung | DPL, FW, VW | — |
| FW  | Leistung über Frequenz | Wirkleistung | DPL, APC, VW | — |
| VW  | Leistung über Spannung | Wirkleistung | DPL, APC, FW | — |
| SPF | fester Leistungsfaktor | Blindleistung | eine Wirkleistungs-Funktion | WPF, Volt-Var, RPC |
| WPF | Leistungsfaktor über Leistung | Blindleistung | eine Wirkleistungs-Funktion | SPF, Volt-Var, RPC |
| Volt-Var / CC | Blindleistung über Spannung | Blindleistung | eine Wirkleistungs-Funktion | SPF, WPF, RPC |
| RPC | feste Blindleistung | Blindleistung | eine Wirkleistungs-Funktion | SPF, WPF, Volt-Var |
| ID  | Trennung bei Netzausfall | Schutz | alles | — (überschreibt bei Auslösung alles) |

Hinweise:
- Wirkleistungs-Funktionen widersprechen sich nie — sie überlagern sich, und die strengste Grenze
  gilt.
- Die vier Blindleistungs-Funktionen schließen sich gegenseitig aus. Ein Netzprofil aktiviert
  normalerweise genau eine; wären mehrere gleichzeitig aktiv, wendet der Wechselrichter
  trotzdem nur EINE an — nach fester Priorität: **Volt-Var, dann Watt-Power-Factor, dann
  fester Leistungsfaktor (SPF), dann Blindleistungssteuerung (RPC), dann ein fünfter
  kurvenbasierter Blindleistungsmodus** — die übrigen werden ignoriert. Sie summieren sich nie.
- Wirk- und Blindleistung teilen sich die Gesamtleistung des Geräts; bei sehr hoher Ausgangs-
  leistung ist daher weniger Blindleistungs-Stützung möglich (und umgekehrt).
- Die Inselnetz-Erkennung ist ein unabhängiger Schutz; wenn sie trennt, stoppt jede Einspeisung.

## Gut zu wissen (wie sich der Wechselrichter tatsächlich verhält)

- **Jede Funktion wird einzeln konfiguriert** im Netzprofil — eigene An/Aus-Schaltung plus eigene
  Schwellen bzw. Kennlinie. Ein Profil nutzt daher eine beliebige Kombination der Wirkleistungs-
  Funktionen zusammen mit genau einem Blindleistungsmodus.
- **Die strengste Wirkleistungsgrenze gewinnt immer.** Wirken z. B. eine Einspeisegrenze (DPL) und
  eine Überfrequenz-Absenkung (FW) gleichzeitig, folgt der Wechselrichter automatisch derjenigen,
  die weniger zulässt — ohne Konflikt zwischen beiden.
- **Grenzen werden weich angewandt, nicht abrupt** — kleine Schritte mit kurzer Beruhigungszeit —
  weshalb die Leistung bei einer neuen Grenze rampt statt zu springen.
- **Blindleistungs-Stützung tritt bei Volllast hinter die Wirkleistung zurück**, weil beide auf
  dieselbe Gesamtleistung zugreifen; sobald die Wirkleistung sinkt, entsteht mehr Reserve für
  Spannungs-/Leistungsfaktor-Stützung.

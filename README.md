# Vorbereitung

Zur Nutzung des Routers müssen OSM-Karten im .osm-Format vorliegen.

## Herunterladen der Karten
z.B. unter [https://download.geofabrik.de/europe/germany.html](https://download.geofabrik.de/europe/germany.html)

## Konvertierung der Daten
```bash
osmconvert <dateiname.osm.pbf> -o=<dateiname>.osm
```

## Filtern der Daten
Das Filtern wird zur Reduktion des Arbeitsspeicherbedarfs dringend empfohlen!
```bash
osmfilter <dateiname>.osm \
  --keep="highway=*" \
  --drop="area=yes" \
  --drop-relations \
  --drop-author \
  --drop-version \
  > <dateiname_gefiltert>.osm
```

# Nutzung

Start des Routers mit `./router_app <dateiname_gefiltert>.osm`.

Der Start kann je nach Kartengröße mehrere Minuten in Anspruch nehmen. Wichtig: Es sollte immer 5x mehr freier RAM als die Karte groß ist zur Verfügung stehen.

Nach der Initialisierung können zwei OSM-Node-IDs mit Leerzeichen getrennt angegeben werden: `3090980390 33122434`. Die Route wird daraufhin mit fortlaufendem Index im Programmverzeichnis als GeoJSON gespeichert.

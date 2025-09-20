-  Schreibe Unit Tests
- Ich kann auch noch den 'help' output sortieren - also von a - z
- Ich könnte auch noch die Funktionen dynamisch hinzufügen lassen - der ganze Speicher muss dafür schon verher geblockt sein
- Das wäre auch sinnvoll für die help function, weil ich diese dann in der Init Funktion hinzufügen kann, und die callbacks wirklich keine globale Variable brauchen.
- Autocompletion wäre auch noch sehr praktisch.
- Fuzzing wäre noch spannend für diese CLI - wie kann man das machen?


# Zukünftige Arbeitspakete
- Ich will hier auch einen Tree bauen können - also zum Beispiel
```
help
* test
        enter 'help test' to display all available tests
* datamodel
        enter 'help datamodel' to display all available datamodel nodes
...
```

```
help test
* led on
        enables the blinky led on PA5
* led off
        disables the blinky led
...
```

Before you start translating pages, briefly read the rules:
1. the pattern is in the file “en.lang”
2. one entry looks as follows: variable.storing.text: “example text”
3. the QML pages are separated by a newline character (“\n”) 
4. the QML pages are arranged in the correct order (the order is written out at the end of this file)
5. the entries in these QML pages are also in the correct order

How to implement new language in program:
	Once you have the localization file ready, in order for it to be loaded by the program, you need to add the name of the language in the “Settings” class in the “availableLanguages” variable, and in the “availableLanguagesToString” function add a reference to this file.
	Now all you need to do is to add in “Options.qml” a RadioButton in the same way as it was done with the existing language options.
	In the onClicked function, specify Settings.setLanguage(language_number) and you're done.
	
List of QML pages (which indicates running order):
0. "Main.qml"
1. "Options.qml"
2. "inputHamming.qml"
3. "HammingGenerationMatrix.qml"
4. "Hamming.qml"
5. "HammingSyndrome.qml"
6. "InputReedSolomon.qml"
7. "Galois.qml"
8. "ReedSolomon.qml"
# Ανάπτυξη Λογισμικού για Πληροφοριακά Συστήματα
## Application: Project_2019_1
## File: README.md  
### Team Members
Γεωργακόπουλος Παναγιώτης 1115201600028\
Καραμηνά Μαρία            1115201600059\
Κουρσιούνης Γεώργιος      1115201600077
### Instructor
Πασκαλής Σαράντης
### Tests
Τα CUnit tests εκτελέστηκαν στους προσωπικούς μας υπολογιστές κατόπιν αδυναμίας εκτέλεσης τους στα μηχανήματα του Τμήματος

### 1) Λίγα λόγια για την υλοποίηση της εργασίας ανά αρχείο
Η υλοποίηση της εργασίας έγινε σε γλώσσα C (C99) και αποτελείται από δεκαπέντε (15) αρχεία:\
    1) main.c\
    2) quicksort.c\
    3) quicksort.h\
    4) radix_sort.c\
    5) radix_sort.h\
    6) relation.c\
    7) relation.h\
    8) result_list.c\
    9) result_list.h\
    10) sort_merge_join.c\
    11) sort_merge_join.h\
    12)
    13) main.c:    
    14) Makefile:  Ένα απλό Makefile για την μεταγλώττιση των προγραμμάτων.
    15) README.md: Αυτό εδώ το ReadMe.
    
### 2) Διαφορές με την εκφώνηση και κάποιες σχεδιαστικές επιλογές

### 3) Μεταγλώττιση και χρήση των προγραμμάτων
Έχουμε θεωρήσει πως τα προγράμματα θα εκτελεστούν σύμφωνα με τις οδηγίες αυτού εδώ του εγγράφου
και για αυτό δεν έχουμε καλύψει κάθε ενδεχόμενο λάθους χρήσης, καθώς αυτό δεν ήταν το ζητούμενο της εργασίας.\
Για την μεταγλώττιση στα μηχανήματα των Linux κατεβάζετε τα αρχεία από το git και χρησιμοποιείτε τις εντολές:\
    1) Για το join: make ή make all\
    2) Για τα tests: make tests\
    3) Για διαγραφή των εκτελέσιμων και των αντικειμενικών (.o) αρχείων: make clean (ΣΗΜΕΊΩΣΉ: θα διαγραφούν όλα τα *.o του φακέλου)\
    
##### Εκτέλεση:\
Μετά την μεταγλώττιση η εντολή για την εκκίνηση της εργασίας είναι: 
 ./join   path_to_relation_input1   path_to_relation_input2  

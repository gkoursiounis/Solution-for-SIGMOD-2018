/******************************************************************************/
/*  Application         :    Project_2019_1                                   */
/*  File                :    README.md                                        */
/*  Team Members                                                              */
/*    Georgakopoulos Panagiotis 1115201600028                                 */
/*    Karamhna Maria            1115201600059                                 */
/*    Koursiounis Georgios      1115201600077                                 */
/*  Instructor          :    Sarantis Paskalis                                */
/*  Most Tests Conducted At The University's Linux Machines                   */
/*  The CUnit Tests Conducted At Our Personal Machines                        */
/******************************************************************************/

1) Λίγα λόγια για την υλοποίηση της εργασίας ανά αρχείο.
2) Διαφορές με την εκφώνηση και κάποιες σχεδιαστικές επιλογές.
3) Μεταγλώττιση και χρήση των προγραμμάτων.

1) Λίγα λόγια για την υλοποίηση της εργασίας ανά αρχείο.
    Η υλοποίηση της εργασίας έγινε σε γλώσσα C (C99) και αποτελείται από δεκαπέντε (15) αρχεία:
    1) 
    2) 
    3) 
    4) 
    5) 
    6) 
    7) 
    8) 
    9) 
    10)
    11)
    12)
    13) main.c:    
    14) Makefile:  Ένα απλό Makefile για την μεταγλώττιση των προγραμμάτων.
    15) README.md: Αυτό εδώ το ReadMe.
2) Διαφορές με την εκφώνηση και κάποιες σχεδιαστικές επιλογές:
i) 
ii)

3) Μεταγλώττιση και χρήση των προγραμμάτων.
    Έχουμε θεωρήσει πως τα προγράμματα θα εκτελεστούν σύμφωνα με τις οδηγίες αυτού εδώ του εγγράφου
    και για αυτό δεν έχουμε καλύψει κάθε ενδεχόμενο λάθους χρήσης, καθώς αυτό δεν ήταν το ζητούμενο της
    εργασίας.
    Για την μεταγλώττιση στα μηχανήματα των Linux κατεβάζετε τα αρχεία από το git και χρησιμοποιείτε τις εντολές:
        1) Για το join: make ή make all
        2) Για τα tests: make tests
        Για διαγραφή των εκτελέσιμων και των αντικειμενικών (.o) αρχείων: make clean
        ΣΗΜΕΊΩΣΉ: θα διαγραφούν όλα τα *.o του φακέλου.
    Οδηγίες χρήσης της Εργασίας:
    Μετά την μεταγλώττιση η εντολή για την εκκίνηση της εργασίας είναι ./join path_to_relation_input1 path_to_relation_input2  

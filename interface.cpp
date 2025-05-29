#include <gtk/gtk.h> 
#include <stdlib.h> 
#include <string>

//pointeurs vers des éléments d'interface
GtkWidget *menu_mode; //liste déroulante principale qui permet de choisir Suivi couleur / Robot Police
GtkWidget *menu_couleur; 
GtkWidget *menu_teeshirt; 
GtkWidget *menu_masque;
GtkWidget *box_couleur;
GtkWidget *box_robot; //conteneur avec les différents éléments associés 

std::string mode = "Suivi Couleur"; //Le mode seléctionné par dééfaut


void chgmt_mode(GtkComboBox *widget, gpointer data) //callback déclenché quand "changed" activé
{
    gchar *choix;
    choix = gtk_combo_box_text_get_active_text(GTK_COMBO_BOX_TEXT(widget)); //On récupère la valeur choisie dans la liste déroulante chosie

    if (choix == NULL)
    {
        return; //Si on ne sélectionne aucun mode on ne fait rien
    }
        
    mode = choix; // on met la valeur chosie dans mode

    g_print("Mode sélectionné = %s\n", choix);//affiche directement dans le terminal au lieu d'ouvrir les fichiers 
    g_free(choix); 

    if (mode == "Suivi Couleur") //Si suivi de couleur est choisie
    {
        gtk_widget_show(box_couleur); //On affiche les couleurs possibles
        gtk_widget_hide(box_robot); //On cache les options liées au robot
    }

    else //Si c'est Robot police inversement
    {
        gtk_widget_hide(box_couleur);  
        gtk_widget_show(box_robot);
    }
}

void on_lancer_button(GtkButton *button, gpointer user_data) //Fonction callback execute lors du signal "clicked"
{
    if (mode == "Suivi Couleur")
    {
        gchar *couleur;
        couleur = gtk_combo_box_text_get_active_text(GTK_COMBO_BOX_TEXT(menu_couleur)); //on prend la couleur sélectionné
        g_print("couleur = %s\n", couleur);

        if (couleur != NULL)
        {
            FILE *file;
            file = fopen("couleur.txt", "w");
            if(file != NULL)
            {
                fprintf(file, "%s\n", couleur);
                fclose(file);
            }
            system("./camera_red_detect &");
            g_free(couleur);
        }
    }

    else //si mode robot police
    {
        gchar *teeshirt;
        gchar *masque;
        teeshirt = gtk_combo_box_text_get_active_text(GTK_COMBO_BOX_TEXT(menu_teeshirt)); //sélection du teeshirt récupéré + masque
        masque = gtk_combo_box_text_get_active_text(GTK_COMBO_BOX_TEXT(menu_masque));
        g_print("teeshirt = %s, masque = %s\n", teeshirt, masque);
        
        
        if (teeshirt != NULL && masque != NULL)
        {
            //On crée le fichier config.txt qui contient les paramètres qui permettent de détecter l'individu recherché
            FILE *file;
            file = fopen("config.txt", "w");
            if (file != NULL)
            {
                fprintf(file, "tshirt : %s\n", teeshirt);
                fprintf(file, "masque : %s\n", masque);
                fclose(file);
            }
            g_free(teeshirt);
            g_free(masque);

            //On lance le programme RobotPolice
            system("./robot_police_cam_mac &");
        }
    }
}

int main(int argc, char *argv[]) 
{ 
    GtkWidget *window;
    GtkWidget *pp_box;
    GtkWidget *button_lancer;

    gtk_init(&argc, &argv); //initialisation gtk

    //Création de la fenetre principale (titre et taille de la fenetre)
    window = gtk_window_new(GTK_WINDOW_TOPLEVEL);
    gtk_window_set_title(GTK_WINDOW(window), "Panneau de Contrôle");
    gtk_window_set_default_size(GTK_WINDOW(window), 500, 500);

    //Connexion du signal de fermeture de la fenetre au fait de quitter le programme
    g_signal_connect(window, "destroy", G_CALLBACK(gtk_main_quit), NULL);

    //Création d'une verticale box qu'on ajoute à la fenetre principale
    pp_box = gtk_box_new(GTK_ORIENTATION_VERTICAL, 15);
    gtk_container_add(GTK_CONTAINER(window), pp_box);

    //Label du mode choisi par l'utilisateur
    GtkWidget *label_mode;
    label_mode = gtk_label_new("Sélectionnez le mode de fonctionnement :");
    //Insertion widget dans la box pp
    gtk_box_pack_start(GTK_BOX(pp_box), label_mode, FALSE, FALSE, 15);

    //On crée le menu déroulant principal en ajoutant les deux options possibles
    menu_mode = gtk_combo_box_text_new(); 
    
    gtk_combo_box_text_append_text(GTK_COMBO_BOX_TEXT(menu_mode), "Suivi Couleur");
    gtk_combo_box_text_append_text(GTK_COMBO_BOX_TEXT(menu_mode), "RobotPolice"); 

    gtk_combo_box_set_active(GTK_COMBO_BOX(menu_mode), 0); //par défaut on a suivi de couleur au démarrage
    g_signal_connect(G_OBJECT(menu_mode), "changed", G_CALLBACK(chgmt_mode), NULL); //on appelle changed à chgmt mode
    gtk_box_pack_start(GTK_BOX(pp_box), menu_mode, FALSE, FALSE, 5);

    //Box verticale pour les différentes couleurs 
    box_couleur = gtk_box_new(GTK_ORIENTATION_VERTICAL, 5);

    GtkWidget *label_couleur;
    label_couleur = gtk_label_new("La couleur à suivre est :");
    gtk_box_pack_start(GTK_BOX(box_couleur), label_couleur, FALSE, FALSE, 5);


    //On a les couleurs affichées dans le menu
    menu_couleur = gtk_combo_box_text_new();
    gtk_combo_box_text_append_text(GTK_COMBO_BOX_TEXT(menu_couleur), "rouge");
    gtk_combo_box_text_append_text(GTK_COMBO_BOX_TEXT(menu_couleur), "vert");
    gtk_combo_box_text_append_text(GTK_COMBO_BOX_TEXT(menu_couleur), "bleu");

    gtk_combo_box_set_active(GTK_COMBO_BOX(menu_couleur), 0);//la box couleur ne sera affiché qie si le mode suivi de couleur est choisie
    gtk_box_pack_start(GTK_BOX(box_couleur), menu_couleur, FALSE, FALSE, 5);
    gtk_box_pack_start(GTK_BOX(pp_box), box_couleur, FALSE, FALSE, 5);

    //Box robot et ses paramètres
    box_robot = gtk_box_new(GTK_ORIENTATION_VERTICAL, 5);

    GtkWidget *label_teeshirt;
    label_teeshirt = gtk_label_new("La couleur du tee-shirt est :");
    gtk_box_pack_start(GTK_BOX(box_robot), label_teeshirt, FALSE, FALSE, 5);

    //menu avec les options de couleur du teeshirt
    menu_teeshirt = gtk_combo_box_text_new();
    gtk_combo_box_text_append_text(GTK_COMBO_BOX_TEXT(menu_teeshirt), "rouge");
    gtk_combo_box_text_append_text(GTK_COMBO_BOX_TEXT(menu_teeshirt), "bleue");
    gtk_combo_box_text_append_text(GTK_COMBO_BOX_TEXT(menu_teeshirt), "noire");
    gtk_combo_box_text_append_text(GTK_COMBO_BOX_TEXT(menu_teeshirt), "blanc");
    gtk_combo_box_text_append_text(GTK_COMBO_BOX_TEXT(menu_teeshirt), "grise");
    gtk_combo_box_text_append_text(GTK_COMBO_BOX_TEXT(menu_teeshirt), "vert");
    gtk_combo_box_text_append_text(GTK_COMBO_BOX_TEXT(menu_teeshirt), "jaune");
    
    gtk_combo_box_set_active(GTK_COMBO_BOX(menu_teeshirt), 0);
    gtk_box_pack_start(GTK_BOX(box_robot), menu_teeshirt, FALSE, FALSE, 5);

    GtkWidget *label_masque; 
    label_masque = gtk_label_new("Le suspect a-t-il un masque ?");
    gtk_box_pack_start(GTK_BOX(box_robot), label_masque, FALSE, FALSE, 5);
    
    //Le suspect a-t-il un masque ?
    menu_masque = gtk_combo_box_text_new();
    gtk_combo_box_text_append_text(GTK_COMBO_BOX_TEXT(menu_masque), "oui");
    gtk_combo_box_text_append_text(GTK_COMBO_BOX_TEXT(menu_masque), "non");

    gtk_combo_box_set_active(GTK_COMBO_BOX(menu_masque), 0);
    gtk_box_pack_start(GTK_BOX(box_robot), menu_masque, FALSE, FALSE, 5);
    gtk_box_pack_start(GTK_BOX(pp_box), box_robot, FALSE, FALSE, 5);
    
    //Création du bouton Lancer
    button_lancer = gtk_button_new_with_label("Lancer");
    g_signal_connect(G_OBJECT(button_lancer), "clicked", G_CALLBACK(on_lancer_button), NULL);
    gtk_box_pack_start(GTK_BOX(pp_box), button_lancer, FALSE, FALSE, 10); 

    gtk_widget_show_all(window); //on affiche tous les éléments de la fenêtre
    gtk_widget_hide(box_robot); //la box est caché et montré que si mode robot

    gtk_main();

    return EXIT_SUCCESS;
}
 
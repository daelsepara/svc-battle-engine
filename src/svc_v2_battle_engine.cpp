#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <conio.h>

#include "Capcom_chars_hp.h"
#include "Capcom_chars_sp.h"
#include "SNK_chars_hp.h"
#include "SNK_chars_sp.h"
#include "SNK_backups.h"
#include "Capcom_backups.h"

int combat_area[2][3];              // character cards in play
int combat_area_card_type[2][3];    // card types (cards in play)
int combat_area_card_hp[2][3];      // card hp (cards in play)
int combat_ready[2][3];             // card combat readiness
int freeze_status[2][3];            // card freeze status
int attack_status[2][3];            // card attack status
int backup_status[2][3];            // backup status
int combat_area_backups[2][3][3];   // backups of combat area characters
int num_backups[2][3];              // number of backups supporting a combat area
                                    // character
                                    
int combat_decks[2][50];            // player decks
int card_types[2][50];              // card type (cards in player decks)
int hand_cards[2][50];              // player cards in hand
int hand_card_type[2][50];          // player cards in hand (type)
int discard_pile_cards[2][50];      // player cards in discard pile
int discard_pile_card_type[2][50];  // player cards in discard pile (type)
int hand_size[2];
int deck_ptr[2];
int discard_pile_ptr[2];

int deploy_status[2];    // flags to determine whether a player
                                // has already deployed a character

int computer_block_pos[3];     // Computer A.I. Block Moves
int human_block_pos[3];        // Human Block Moves      

int player_hp[2];           // player HP
int player_sp[2];           // player SP

const int BOTH_PLAYERS = 2;
const int COMPUTER_PLAYER = 1;
const int HUMAN_PLAYER = 0;
const int NUM_CARDS = 50;
const int MAX_HAND_SIZE = 50;

const int SNK_CARD = 0;
const int CAPCOM_CARD = 1;
const int ACTION_CARD = 2;

void build_random_deck(int player)
{
    srand(time(0));

    printf("Building deck for player %d...\n",player);    

    for (int i=0;i<NUM_CARDS;i++)
    {
        
        int card = (int)(rand()%120);
        
        printf("card #%d(%d) selected\n",card,player);
                                    
        combat_decks[player][i] = card;
        card_types[player][i] = player;
        hand_cards[player][i] = 255;
        hand_card_type[player][i] = 255;
    }
    
    deck_ptr[player] = 0;
    
    printf("Done!\n\n");    
}

void init_discard_pile(int player)
{
    for (int i=0;i<NUM_CARDS;i++)
    {
        discard_pile_cards[player][i] = 255;
        discard_pile_card_type[player][i] = 255;
    }
    
    discard_pile_ptr[player] = 0;
}

void put_to_discard_pile(int player,int card_pos)
{
    discard_pile_cards[player][discard_pile_ptr[player]] = combat_area[player][card_pos];
    discard_pile_card_type[player][discard_pile_ptr[player]] = combat_area_card_type[player][card_pos];
    discard_pile_ptr[player]++;  
}

void kill_character(int player, int player_pos)
{
    combat_area_card_hp[player][player_pos] = 0;
    combat_area[player][player_pos] = 255;
    combat_area_card_type[player][player_pos] = 255;
}

int draw_card(int player)
{
    if (deck_ptr[player] == MAX_HAND_SIZE) return 0;
    
    int hand_ptr = 0;
    
    while(hand_ptr<MAX_HAND_SIZE)
    {
        if (hand_cards[player][hand_ptr] == 255)
        {
                hand_cards[player][hand_ptr] = combat_decks[player][deck_ptr[player]];
                hand_card_type[player][hand_ptr] = card_types[player][deck_ptr[player]];
                break;
        }
        else
        {
                hand_ptr++;
        }
    }
    
    deck_ptr[player]++;
    return 1;    
}

void count_hand(int player)
{
    hand_size[player] = 0;
    
    for (int i=0; i<MAX_HAND_SIZE; i++)
    {
        if (hand_cards[player][i]!=255) hand_size[player]++;
    }
}

void init_hand(int player)
{
    for (int i=0; i<5; i++)
    {
        draw_card(player);
    }
}

void clear_deploy_status(int player)
{
    deploy_status[player] = 0;
}

void clear_backup_status(int player)
{
    for (int i=0;i<3; i++) backup_status[player][i] = 0;
}

void set_deploy_status(int player)
{
    deploy_status[player] = 1;
}

void clear_combat_area(void)
{
    for (int y = 0;y<2;y++)
    {
        for (int x = 0;x<3;x++)
        {
                combat_area[y][x] = 255;
                combat_area_card_type[y][x] = 255;
                freeze_status[y][x] = 0;
                attack_status[y][x] = 0;
                backup_status[y][x] = 0;
                num_backups[y][x] = 0;
        }
    }
}

int get_hp(int player, int pos)
{
    if (combat_area[player][pos]==255) return 0; else return (combat_area_card_hp[player][pos]);
}

void print_combat_area_card(int player, int pos)
{
    if (combat_area[player][pos]==255) return; else 
    
    if (combat_area_card_type[player][pos] == SNK_CARD) printf("[SNK  "); else printf("[CAPCOM ");
    printf("#%d HP: %d]",combat_area[player][pos],get_hp(player,pos));
}   

void view_combat_area(int player)
{
    printf("\n");
    
    if (player == COMPUTER_PLAYER) printf("COMPUTER:\n"); else printf("PLAYER:\n"); 
    
    printf("HP: %d SP: %d\n",player_hp[player],player_sp[player]);
    for (int i=0; i<3; i++)
    {
        if (combat_area_card_type[player][i] == 255) printf("[EMPTY] ");

        int card_num = combat_area[player][i];
        
        if (combat_area_card_type[player][i] == SNK_CARD)
        {
                printf("[SNK %3d] ",card_num);        
        }

        if (combat_area_card_type[player][i] == CAPCOM_CARD)
        {
                printf("[CAPCOM %3d] ",card_num);        
        }
    }

    printf("\n");
    
}

void clear_freeze_status(int player)
{
    for (int x = 0;x<3;x++)
    {
        freeze_status[player][x] = 0;
    }
}

void clear_attack_status(int player)
{
    for (int x = 0;x<3;x++)
    {
        attack_status[player][x] = 0;
    }
}

void make_combat_ready(int player)
{
    for (int x = 0;x<3;x++)
    {
        combat_ready[player][x] = 1;
    }
}

int place_card(int pos, int player, int card_num)
{
    if (combat_area[player][pos]!=255) return 0;
    if (hand_card_type[player][card_num] == ACTION_CARD) return 0;
    
    combat_area[player][pos] = hand_cards[player][card_num]; 
    combat_area_card_type[player][pos] = hand_card_type[player][card_num];

    if (hand_card_type[player][card_num] == SNK_CARD)
    {
        combat_area_card_hp[player][pos] = SNK_chars_hp[hand_cards[player][card_num]];
        player_sp[player] += SNK_chars_sp[hand_cards[player][card_num]];
        
        // copy backup info
        for (int i=0; i<3; i++)
        {
             combat_area_backups[player][pos][i] = SNK_backups[hand_cards[player][card_num]*3+i];
        }
        
        backup_status[player][pos] = 1;
        num_backups[player][pos] = 0;
    }
    
    if (hand_card_type[player][card_num] == CAPCOM_CARD)
    {
        combat_area_card_hp[player][pos] = Capcom_chars_hp[hand_cards[player][card_num]];
        player_sp[player] += Capcom_chars_sp[hand_cards[player][card_num]];  

        // copy backup info
        for (int i=0; i<3; i++)
        {
             combat_area_backups[player][pos][i] = Capcom_backups[hand_cards[player][card_num]*3+i];
        }
        
        backup_status[player][pos] = 1;
        num_backups[player][pos] = 0;
    }
    
    hand_cards[player][card_num] = 255;
    hand_card_type[player][card_num] = 255;

    freeze_status[player][pos] = 0;
    attack_status[player][pos] = 0;
    combat_ready[player][pos] = 0;
    
    return 1;
}

int check_for_backups(int player, int card_num)
{
    int card_type = hand_card_type[player][card_num];
    int char_num = hand_cards[player][card_num];
    int backup = 0;

    if (card_type == ACTION_CARD) return 0;

    for (int i=0;i<3; i++)
    {
        for (int j = 0; j<3; j++)
        {
                if (card_type == CAPCOM_CARD)
                {
                     if (!backup_status[player][i]&&combat_area[player][i]!=255&&(combat_area_backups[player][i][j]  == 120 + char_num))
                     {
                          backup = i+1;
                     }
                }
                else
                {
                     if (!backup_status[player][i]&&combat_area[player][i]!=255&&(combat_area_backups[player][i][j] == char_num))
                     backup = i+1;
                }
        }
    }
    
    return backup;
}

int check_backup(int player, int card_num, int backup_pos)
{
    if (backup_status[player][backup_pos]) return 0;
    if (combat_area[player][backup_pos] == 255) return 0;
    
    int card_type = hand_card_type[player][card_num];
    int char_num = hand_cards[player][card_num];
    
    if (card_type == ACTION_CARD) return 0;
    
    int backup = 0;
    
    for (int i=0; i<3; i++)
    {
        if (card_type == CAPCOM_CARD)
        {
             if (combat_area_backups[player][backup_pos][i] == 120 + char_num) backup = backup_pos + 1;
        }
        else
        {
             if (combat_area_backups[player][backup_pos][i] == char_num) backup = backup_pos + 1;
        }
    }
    
    return backup;
}

void do_backup(int player, int card_num, int backup_pos)
{
    if (backup_status[player][backup_pos]) return;
    if (combat_area[player][backup_pos] == 255) return;
    
    int card_type = hand_card_type[player][card_num];
    int char_num = hand_cards[player][card_num];

    if (card_type == ACTION_CARD) return;

    for (int i=0; i<3; i++)
    {
        if (card_type == CAPCOM_CARD)
        {
             if (combat_area_backups[player][backup_pos][i] == 120 + char_num)
             {
                  backup_status[player][backup_pos] = 1;
                  combat_area_card_hp[player][backup_pos] += 300;
                  combat_area_backups[player][backup_pos][i] = 255;
                  hand_cards[player][card_num] = 255;
                  hand_card_type[player][card_num] = 255;
                  print_combat_area_card(player,backup_pos);
                  printf(" Powered!\n");
                  num_backups[player][backup_pos]++;
                  break;
             }
        }
        else
        {
             if (combat_area_backups[player][backup_pos][i] == char_num)
             {
                  backup_status[player][backup_pos] = 1;
                  combat_area_card_hp[player][backup_pos] += 300;
                  combat_area_backups[player][backup_pos][i] = 255;
                  hand_cards[player][card_num] = 255;
                  hand_card_type[player][card_num] = 255;
                  print_combat_area_card(player,backup_pos);
                  printf(" Powered!\n");
                  num_backups[player][backup_pos]++;
                  break;
             }
        }
    }
}

void use_special_card(int player, int card)
{

}

void do_computer_use_special_card()
{

}

void do_special_effects(int player, int card_pos)
{
// performs special effects as cards enter or leave play
}

void do_end_of_turn_effects(int player)
{
// performs end of turn effects
// calls do_special_effects
}

int can_block(int player, int pos)
{
    if (combat_area[player][pos]!=255&&!attack_status[player][pos]&&!freeze_status[player][pos]) return 1; else return 0;
}

int can_attack(int player, int pos)
{
    if (combat_area[player][pos]!=255&&!attack_status[player][pos]&&!freeze_status[player][pos]&&combat_ready[player][pos]) return 1; else return 0;
}

int card_battle(int player, int attack_pos, int block_pos)
{
    int opponent = 1 - player;

    attack_pos --;   // convert 1-3 to 0-2

    int player_card_hp = combat_area_card_hp[player][attack_pos];
    freeze_status[player][attack_pos] = 1;

    if (block_pos == 255)
    {
        player_hp[opponent] -= player_card_hp;
        if (player_hp[opponent]<0) player_hp[opponent] = 0;
        return player;
    }

    block_pos --;    // convert 1-3 to 0-2

    freeze_status[opponent][block_pos] = 1;
    
    int opponent_card_hp = combat_area_card_hp[opponent][block_pos];
                  
    // perform actual card combat
    combat_area_card_hp[player][attack_pos] -= opponent_card_hp;
    combat_area_card_hp[opponent][block_pos] -= player_card_hp;
    
    if (combat_area_card_hp[player][attack_pos]<=0)
    {
        put_to_discard_pile(player,attack_pos);
        kill_character(player,attack_pos);
    }
    
    if (combat_area_card_hp[opponent][block_pos]<=0)
    {
        put_to_discard_pile(opponent,block_pos);
        kill_character(opponent,block_pos);
    }

    if (!combat_area_card_hp[player][attack_pos]&&!combat_area_card_hp[opponent][block_pos])
    {
        printf("Both cards destroyed!\n");
        return BOTH_PLAYERS; 
    }
    
    if (!combat_area_card_hp[player][attack_pos]) 
    {
        if (player == HUMAN_PLAYER)
        {
                printf("Your card is destroyed!\n");
        }
        else
        {
                printf("Computer's card destroyed!\n");
        }
        
        return opponent;
    }
    
    if (player == HUMAN_PLAYER)
    {
        printf("Computer's card destroyed!\n");
    }
    else
    {
        printf("Your card is destroyed!\n");
    }
    
    attack_status[player][attack_pos] = 1; 
    return player;
}

int united_attack(int player, int attack_pos1, int attack_pos2, int block_pos)
{
    int opponent = 1 - player;

    player_sp[player] -= 5;

    attack_pos1 --;
    attack_pos2 --;
    
    freeze_status[player][attack_pos1] = 1;
    freeze_status[player][attack_pos2] = 1;
    int player_card_hp1 = get_hp(player,attack_pos1);
    int player_card_hp2 = get_hp(player,attack_pos2);   
    int total_player_card_hp = player_card_hp1+player_card_hp2;
    
    if (block_pos == 255)
    {
        player_hp[opponent] -= total_player_card_hp;
        if (player_hp[opponent]<0) player_hp[opponent] = 0;
        
        return player;   
    }
    
    block_pos --;
    
    freeze_status[opponent][block_pos] = 1;
    
    int opponent_card_hp = combat_area_card_hp[opponent][block_pos];             

    // perform actual card combat
    int united_attack_hp = total_player_card_hp - opponent_card_hp;
    combat_area_card_hp[opponent][block_pos] -= total_player_card_hp;
    
    if (combat_area_card_hp[opponent][block_pos]<=0)
    {
        put_to_discard_pile(opponent,block_pos);
        kill_character(opponent,block_pos);
    }
    
    if (united_attack_hp<=0)
    {
        put_to_discard_pile(player,attack_pos1);
        put_to_discard_pile(player,attack_pos2);
        kill_character(player,attack_pos1);
        kill_character(player,attack_pos2);
    }
    else    
    {
        player_hp[opponent] -= united_attack_hp;
        if (player_hp[opponent]<0) player_hp[opponent] = 0;

        attack_status[player][attack_pos1] = 1;
        attack_status[player][attack_pos2] = 1;
        
        if (opponent_card_hp>=player_card_hp2)
        {
                put_to_discard_pile(player,attack_pos2);
                kill_character(player,attack_pos2);
                combat_area_card_hp[player][attack_pos1] = united_attack_hp;
        }
        else
        {
                combat_area_card_hp[player][attack_pos2] = player_card_hp2 - opponent_card_hp;
        }
    }
    
    if (!combat_area_card_hp[player][attack_pos1]&&!combat_area_card_hp[opponent][block_pos])
    {
        printf("Both cards destroyed!\n");
        return BOTH_PLAYERS;
    }
    
    if (!combat_area_card_hp[player][attack_pos1])
    {
        if (player == HUMAN_PLAYER)
        {
                printf("Your card is destroyed!\n");
        }
        else
        {
                printf("Computer's card destroyed!\n");
        }
        
        return opponent;
    }

    if (player == HUMAN_PLAYER)
    {
        printf("Computer's card destroyed!\n");
    }
    else
    {
        printf("Your card is destroyed!\n");
    }

    return player;
}

int do_computer_place_card()
{
// this performs the "USE card from HAND" for the computer player
// this is composed of two steps:
// first the computer tries to put a character onto the combat area
// --- if it does, it decides between maximizing the attack capabilities
// or maximizing SPs for special actions
// it determines whether or not it can use an action card to give it advantage
//
    int vacant_slots = 0;
    
    for (int i=0;i<3;i++)
    {
        if (combat_area[COMPUTER_PLAYER][i] == 255) vacant_slots++;
    }
    
    // check if there are vacant slots
    if (vacant_slots)
    {
        if (!deploy_status[COMPUTER_PLAYER])
        {
                int max_hp = 0;
                int max_sp = 0;
                int max_hp_card = 0;
                int max_sp_card = 0;
                
                for (int i=0; i<MAX_HAND_SIZE; i++)
                {
                    if ((hand_cards[COMPUTER_PLAYER][i]!=255)&&hand_card_type[COMPUTER_PLAYER][i]==SNK_CARD||hand_card_type[COMPUTER_PLAYER][i]==CAPCOM_CARD)
                    {
                         if (hand_card_type[COMPUTER_PLAYER] == SNK_CARD)
                         {
                              int char_hp = SNK_chars_hp[hand_cards[COMPUTER_PLAYER][i]];
                              if (char_hp >= max_hp)
                              {
                                   max_hp = char_hp;
                                   max_hp_card = i + 1;
                              }

                              int char_sp = SNK_chars_sp[hand_cards[COMPUTER_PLAYER][i]];
                              if (char_sp >= max_sp)
                              {
                                   max_sp = char_sp;
                                   max_sp_card = i + 1;
                              }
                         }
                         else
                         {
                              int char_hp = Capcom_chars_hp[hand_cards[COMPUTER_PLAYER][i]];
                              if (char_hp >= max_hp)
                              {
                                   max_hp = char_hp;
                                   max_hp_card = i + 1;
                              }

                              int char_sp = Capcom_chars_sp[hand_cards[COMPUTER_PLAYER][i]];
                              if (char_sp >= max_sp)
                              {
                                   max_sp = char_sp;
                                   max_sp_card = i + 1;
                              }
                         }
                    }
                }
                
                if (vacant_slots >= 2)
                {
                     int card_deployed = 0;
                     int deploy_pos = 0;
                     
                     for (int i=0; i<3; i++)
                     {
                          if (!card_deployed&&combat_area[COMPUTER_PLAYER][i] == 255)
                          {
                                card_deployed = 1;
                                deploy_pos = i;
                          }
                     }
                     
                     if (card_deployed)
                     {
                          place_card(deploy_pos,COMPUTER_PLAYER,max_hp_card -1);
                          do_special_effects(COMPUTER_PLAYER,deploy_pos);
                          printf("Computer deploys ");
                          print_combat_area_card(COMPUTER_PLAYER,deploy_pos);
                          printf("\n");
                          set_deploy_status(COMPUTER_PLAYER);
                     }
                }
                else
                {
                     if (player_sp[COMPUTER_PLAYER]<5&&player_hp[COMPUTER_PLAYER]>500)
                     {
                          int card_deployed = 0;
                          int deploy_pos = 0;
                     
                          for (int i=0; i<3; i++)
                          {
                               if (!card_deployed&&combat_area[COMPUTER_PLAYER][i] == 255)
                               {
                                    card_deployed = 1;
                                    deploy_pos = i;
                               }
                          }
                     
                          if (card_deployed)
                          {
                               place_card(deploy_pos,COMPUTER_PLAYER,max_sp_card -1);
                               do_special_effects(COMPUTER_PLAYER,deploy_pos);
                               printf("Computer deploys ");
                               print_combat_area_card(COMPUTER_PLAYER,deploy_pos);
                               printf("\n");
                               set_deploy_status(COMPUTER_PLAYER);
                          }
                     }
                     else
                     {
                          int card_deployed = 0;
                          int deploy_pos = 0;
                     
                          for (int i=0; i<3; i++)
                          {
                               if (!card_deployed&&combat_area[COMPUTER_PLAYER][i] == 255)
                               {
                                    card_deployed = 1;
                                    deploy_pos = i;
                               }
                          }
                     
                          if (card_deployed)
                          {
                               place_card(deploy_pos,COMPUTER_PLAYER,max_hp_card -1);
                               do_special_effects(COMPUTER_PLAYER,deploy_pos);
                               printf("Computer deploys ");
                               print_combat_area_card(COMPUTER_PLAYER,deploy_pos);
                               printf("\n");
                               set_deploy_status(COMPUTER_PLAYER);
                          }
                     }
                }
        }
    }

    if (deploy_status[COMPUTER_PLAYER]) return 1; else return 0;
}

int do_computer_view_hand()
{
// this allows the computer to select which card to play
// it can deploy character cards 
// or it can play action cards

// first check if there are characters that can be given backups
    for (int i=0;i<MAX_HAND_SIZE;i++)
    {
        if (hand_cards[COMPUTER_PLAYER][i]!=255&&hand_cards[COMPUTER_PLAYER][i]!=ACTION_CARD)
        {
             int backups = check_for_backups(COMPUTER_PLAYER,i);
             
             if (backups)
             {
                  if (check_backup(COMPUTER_PLAYER,i,backups-1)) do_backup(COMPUTER_PLAYER,i,backups-1);
             }
        }
    }
    
    // check if there are any useful cards that can be used by the computer
    do_computer_use_special_card();

    if (!deploy_status[COMPUTER_PLAYER]) do_computer_place_card();
    
    return 0;
}

void make_computer_attack(int attack_pos1, int attack_pos2, int attack_pos3, int united_pos1, int united_pos2)
{
// this informs the player of the computer's attack and allows the human to
// choose blockers
    
    int num_blockers = 0;
    int block_status[3];
    
    for (int i=0; i<3; i++)
    {
        human_block_pos[i] = 255;
        block_status[i] = 0;
        if (can_block(HUMAN_PLAYER,i)) num_blockers++;
    }

    if (united_pos1)
    {
        printf("Computer makes a united attack using ");
        print_combat_area_card(COMPUTER_PLAYER,united_pos1-1);
        printf(" and ");
        print_combat_area_card(COMPUTER_PLAYER,united_pos2-1);
        printf("\n");
    }
    
    int attack_pos[3];
    attack_pos[0] = attack_pos1;
    attack_pos[1] = attack_pos2;
    attack_pos[2] = attack_pos3;
    
    for (int i=0;i<3;i++)
    {
        if (attack_pos[i])
        {
                printf("Computer Attacks with ");
                print_combat_area_card(COMPUTER_PLAYER,attack_pos[i]-1);
                printf("\n");
        }
    }
    
    if (!num_blockers)
    {
        printf("You do not have any blockers!\n");
        return;
    }
    
    if (united_pos1)
    {
        int block_flag = 0;
                
        while(!block_flag)
        {
                printf("Block ");
                print_combat_area_card(COMPUTER_PLAYER,united_pos1-1);
                printf(" and ");
                print_combat_area_card(COMPUTER_PLAYER,united_pos2-1);
                printf(" with:\n0 - DO NOT BLOCK\n");
        
                for (int i=0;i<3; i++)
                {
                     if (can_block(HUMAN_PLAYER,i))
                     {
                          printf("%d - ",i+1);
                          print_combat_area_card(HUMAN_PLAYER,i);
                          printf("\n");
                     }
                }
                
                printf("choose blocker:\n");

                char c = getch();
        
                if (c == '0'||c == '1'||c == '2'||c == '3')
                {
                     if (c == '0') block_flag = 1;
                          
                     if (c == '1' || c=='2' || c== '3')
                     {
                          int block_pos = c - '0';
                               
                          if (can_block(HUMAN_PLAYER,block_pos-1))
                          {
                               human_block_pos[1] = block_pos;
                               block_status[block_pos-1] = 1;
                               num_blockers--;
                               block_flag = 1;
                          }
                          else
                          {
                               printf("Can't block!\n");
                          }
                     }
                }
        }
        
        if (attack_pos[0]&&num_blockers)
        {

                int block_flag = 0;
        
                while(!block_flag)
                {
                     printf("Block ");
                     print_combat_area_card(COMPUTER_PLAYER,attack_pos[0]-1);
                     printf(" with:\n0 - DO NOT BLOCK\n");
        
                     for (int i=0;i<3; i++)
                     {
                          if (can_block(HUMAN_PLAYER,i)&&!block_status[i])
                          {
                               printf("%d - ",i+1);
                               print_combat_area_card(HUMAN_PLAYER,i);
                               printf("\n");
                          }
                     }
                
                     printf("choose blocker:\n");

                     char c = getch();
        
                     if (c == '0'||c == '1'||c == '2'||c == '3')
                     {
                          if (c == '0') block_flag = 1;
                          
                          if (c == '1' || c=='2' || c== '3')
                          {
                               int block_pos = c - '0';
                               
                               if (can_block(HUMAN_PLAYER,block_pos-1)&&!block_status[block_pos-1])
                               {
                                     human_block_pos[0] = block_pos;
                                     block_status[block_pos-1] = 1;
                                     num_blockers--;
                                     block_flag = 1;
                               }
                               else
                               {
                                    printf("Can't block!\n");
                               }
                          }
                     }
                }
        }
    }
    else
    {
        for (int j=0;j<3;j++)
        {
             if (attack_pos[j]&&num_blockers)
             {
                  int block_flag = 0;
                  
                  while(!block_flag)
                  {
                       printf("Block ");
                       print_combat_area_card(COMPUTER_PLAYER,attack_pos[j]-1);
                       printf(" with:\n0 - DO NOT BLOCK\n");
        
                       for (int i=0;i<3; i++)
                       {
                            if (can_block(HUMAN_PLAYER,i)&&!block_status[i])
                            {
                                 printf("%d - ",i+1);
                                 print_combat_area_card(HUMAN_PLAYER,i);
                                 printf("\n");
                            }
                       }
                
                       printf("choose blocker:\n");
                       
                       char c = getch();
        
                       if (c == '0'||c == '1'||c == '2'||c == '3')
                       {
                            if (c == '0') block_flag = 1;
                          
                            if (c == '1' || c=='2' || c== '3')
                            {
                                 int block_pos = c - '0';
                               
                                 if (can_block(HUMAN_PLAYER,block_pos-1)&&!block_status[block_pos-1])
                                 {
                                       human_block_pos[j] = block_pos;
                                       block_status[block_pos-1] = 1;
                                       num_blockers--;
                                       block_flag = 1;
                                 }
                                 else
                                 {
                                      printf("Can't block!\n");
                                 }
                            }
                       }
                  }
             }
        }
    }
}

void do_computer_attack()
{
// this determines the attacking actions performed by the computer
// it is not smart --- no strategy is formulated by the computer.
// computer attack action is determined by performing comparisons

    int num_chars = 0;
    int num_attackers = 0;
    
    int num_blockers = 0;
    int num_nonblockers = 0;

    int max_nonblocker_hp = 0;
    int max_blocker_hp = 0;
    
    int max_attacker_hp = 0;
    int min_attacker_hp = 0;
    
    int strongest_attacker = 0;

    int attack_status[3];
    int make_united_attack = 0;
    
    int computer_attack_pos[3];
    int computer_attack_pos1 = 0;
    int computer_attack_pos2 = 0;

    for (int i=0; i<3; i++)
    {
        attack_status[i] = 0;
        computer_attack_pos[i] = 0;
        
        int computer_hp = get_hp(COMPUTER_PLAYER,i);
        int human_hp = get_hp(HUMAN_PLAYER,i);
        
        // determine number of attackers 
        if (can_attack(COMPUTER_PLAYER,i)) 
        {
                num_attackers++;
                if (computer_hp>=max_attacker_hp)
                {
                     max_attacker_hp = computer_hp;
                     strongest_attacker = i+1;
                }
                
                // determine the weakest attacker
                if (computer_hp<=min_attacker_hp||!min_attacker_hp)
                {
                     min_attacker_hp = computer_hp;
                }
        }
        
        // determine characters in human player's combat area
        if (combat_area[HUMAN_PLAYER][i]!=255)
        {
                num_chars++;
                if (!can_block(HUMAN_PLAYER,i))
                {
                     num_nonblockers++;
                     // determine strongest non-blocking character human player's combat area
                     if (human_hp>=max_nonblocker_hp) max_nonblocker_hp = human_hp;
                }
        }
        
        // determine if human player has any blockers
        if (can_block(HUMAN_PLAYER,i))
        {
                num_blockers++;
                if (human_hp>=max_blocker_hp) max_blocker_hp = human_hp;
        }
    }

    // check if an attack is possible
    if (!num_attackers) return;    

    // check if it is a suicidal attack
    if (num_attackers == 1 && num_blockers == 3) return;
    if (num_attackers == 1 && num_chars == 3) return;

    // if there are no blockers in human player's area then proceed to full attack!
    if (!num_chars)
    {
        // attack with all available attackers
        for (int i=0; i<3; i++)
        {
                if (can_attack(COMPUTER_PLAYER,i)) computer_attack_pos[i] = i + 1;
        }
    }
    else
    {
        // there are characters in the area ---
        // proceed cautiously
        
        // check if there are no immediate blockers
        if (num_nonblockers == num_chars)
        {
             // check if computer significantly damage human player and survive a counter attack
             int total_damage = 0;
             int total_counter_damage = 0;
             
             for (int i = 0; i<3; i++)
             {
                  if (combat_area[HUMAN_PLAYER][i]!=255) total_counter_damage += get_hp(HUMAN_PLAYER,i);
                  if (can_attack(COMPUTER_PLAYER,i)) total_damage += get_hp(COMPUTER_PLAYER,i);
             }
             
             // if it can significantly damage human player and survive then proceed with full attack
             if ((total_damage>player_hp[HUMAN_PLAYER]/2)&&total_counter_damage<player_hp[COMPUTER_PLAYER]/2)
             {
                     for (int i=0; i<3; i++)
                     {
                          if (can_attack(COMPUTER_PLAYER,i)) computer_attack_pos[i] = i+1;
                     }
             }
        }
        else
        {
             // there are immediate blockers

             // check if all blockers can be destroyed by the attack
             // regardless of the blocking configurations
             if (num_attackers >= num_blockers)
             {
                  int can_be_destroyed = 1;
                  
                  if (num_attackers == num_blockers)
                  {
                       if (max_blocker_hp>min_attacker_hp)
                       {
                            // not everyone will be destroyed
                            // check if a united attack can be made
                            // so that it can destroy strongest blocker
                            if (player_sp[COMPUTER_PLAYER]>5&&num_attackers==2)
                            {
                                 for (int i=0; i<3; i++)
                                 {
                                      if (can_attack(COMPUTER_PLAYER,i))
                                      {
                                           if (!computer_attack_pos1)
                                           {
                                                computer_attack_pos1 = i+1;
                                           }
                                           else
                                           {
                                                if (!computer_attack_pos2) computer_attack_pos2 = i+1;
                                           }
                                      }
                                 }
                                      
                                 // if it cannot be destroyed do not proceed with attack
                                 if ((get_hp(COMPUTER_PLAYER,computer_attack_pos1-1)+get_hp(COMPUTER_PLAYER,computer_attack_pos2-1)<max_blocker_hp))
                                 {
                                      computer_attack_pos1 = 0;
                                      computer_attack_pos2 = 0;
                                 }
                                 else
                                 {
                                      make_united_attack = 1;
                                      if (get_hp(COMPUTER_PLAYER,computer_attack_pos1-1)>get_hp(COMPUTER_PLAYER,computer_attack_pos2-1))
                                      {
                                           int temp = computer_attack_pos1;
                                           computer_attack_pos1 = computer_attack_pos2;
                                           computer_attack_pos2 = temp;
                                      }
                                 }
                            }
                            else
                            {
                                 // cannot perform a united attack
                                 // check if strongest attacker
                                 // can destroy strongest blocker
                                 if (max_attacker_hp>=max_blocker_hp) computer_attack_pos[0] = strongest_attacker;
                            }
                       }
                       else
                       {
                            // proceed with attack
                            for (int i = 0; i<3; i++)
                            {
                                 if (can_attack(COMPUTER_PLAYER,i)) computer_attack_pos[i] = i+1;
                            }
                       }
                  }
                  else
                  {
                       // computer has more attackers
                       // check if you can make a united attack
                       if (player_sp[COMPUTER_PLAYER]>5)
                       {
                            if (num_attackers == 2)
                            {
                                 for (int i=0; i<3; i++)
                                 {
                                      if (can_attack(COMPUTER_PLAYER,i))
                                      {
                                           if (!computer_attack_pos1)
                                           {
                                                computer_attack_pos1 = i+1;
                                           }
                                           else
                                           {
                                                if (!computer_attack_pos2) computer_attack_pos2 = i+1;
                                           }
                                      }
                                 }
                                      
                                 // if it cannot be destroyed do not proceed with attack
                                 if ((get_hp(COMPUTER_PLAYER,computer_attack_pos1-1)+get_hp(COMPUTER_PLAYER,computer_attack_pos2-1)<max_blocker_hp))
                                 {
                                      computer_attack_pos1 = 0;
                                      computer_attack_pos2 = 0;
                                 }
                                 else
                                 {
                                      make_united_attack = 1;
                                      if (get_hp(COMPUTER_PLAYER,computer_attack_pos1-1)>get_hp(COMPUTER_PLAYER,computer_attack_pos2-1))
                                      {
                                           int temp = computer_attack_pos1;
                                           computer_attack_pos1 = computer_attack_pos2;
                                           computer_attack_pos2 = temp;
                                      }
                                 }
                            }
                            else
                            {
                                 // computer has three attackers
                                 // determine minimum configuration requred to destroy
                                 // strongest blocker
                                 int config[3];
                                 
                                 int min_config_hp = 0;
                                 int min_config = 0;
                                 
                                 config[0] = get_hp(COMPUTER_PLAYER,0)+get_hp(COMPUTER_PLAYER,1);
                                 config[1] = get_hp(COMPUTER_PLAYER,0)+get_hp(COMPUTER_PLAYER,2);
                                 config[2] = get_hp(COMPUTER_PLAYER,1)+get_hp(COMPUTER_PLAYER,2);
                                 
                                 for (int i = 0; i<3; i++)
                                 {
                                      if (config[i]<= min_config_hp||!min_config_hp)
                                      {
                                           min_config_hp = config[i];
                                           min_config = i;
                                      }
                                 }
                                 
                                 if (min_config_hp>=max_blocker_hp&&max_nonblocker_hp<player_hp[COMPUTER_PLAYER]/2)
                                 {
                                      if (min_config == 0) { computer_attack_pos1 = 1; computer_attack_pos2 = 2; }
                                      if (min_config == 1) { computer_attack_pos1 = 1; computer_attack_pos2 = 3; }
                                      if (min_config == 2) { computer_attack_pos1 = 2; computer_attack_pos2 = 3; }
                                      
                                      make_united_attack = 1;
                                      
                                      if (get_hp(COMPUTER_PLAYER,computer_attack_pos1-1)>get_hp(COMPUTER_PLAYER,computer_attack_pos2-1))
                                      {
                                           int temp = computer_attack_pos1;
                                           computer_attack_pos1 = computer_attack_pos2;
                                           computer_attack_pos2 = temp;
                                      }
                                 }
                                 
                                 // if a united attack is feasible check if the other can
                                 // also attack 
                                 if (make_united_attack)
                                 {
                                      int other_attacker = 0;
                                 
                                      for (int i = 0; i<3; i++)
                                      {
                                           if ((computer_attack_pos1!=(i+1))&&(computer_attack_pos2!=(i+1))) other_attacker = i+1;
                                      }
                                 
                                      if (get_hp(COMPUTER_PLAYER,other_attacker-1)>=max_blocker_hp) computer_attack_pos[0] = other_attacker; 
                                 }
                                 else
                                 {
                                      // united attack is not feasible
                                      // determine whether an all out attack can
                                      // eliminate the strongest defender
                                      if (min_attacker_hp>=max_blocker_hp&&max_nonblocker_hp<player_hp[COMPUTER_PLAYER]/2)
                                      {
                                           for (int i = 0; i<3; i++)
                                           {
                                                computer_attack_pos[i] = i+1;
                                           }
                                      }
                                      else
                                      {
                                           // all=out attack is not feasible
                                           // check if strongest attacker
                                           // can take out strongest blocker
                                           if (max_attacker_hp>=max_blocker_hp)
                                           {
                                                computer_attack_pos[0] = strongest_attacker; 
                                           }
                                      }
                                 }
                            }
                       }
                       else
                       {
                            // cannot perform a united attack
                            // check if all-out attack can
                            // take out every blocker
                            if (min_attacker_hp>=max_blocker_hp&&max_nonblocker_hp<player_hp[COMPUTER_PLAYER]/2)
                            {
                                 for (int i = 0; i<3; i++)
                                 {
                                      computer_attack_pos[i] = i+1;
                                 }
                            }
                            else
                            {
                                 // all-out attack is not feasible
                                 // check if strongest attacker
                                 // can take out strongest blocker
                                 if (max_attacker_hp>=max_blocker_hp)
                                 {
                                      computer_attack_pos[0] = strongest_attacker; 
                                 }
                            }
                       }
                  }
             }
             else
             {
                  // computer has fewer attackers
                  // check if an all-out attack is feasible
                  if (min_attacker_hp>=max_blocker_hp&&max_nonblocker_hp<player_hp[COMPUTER_PLAYER]/2)
                  {
                       for (int i = 0; i<3; i++)
                       {
                            if (can_attack(COMPUTER_PLAYER,i)) computer_attack_pos[i] = i+1;
                       }
                  }
                  else
                  {
                       // all=out attack is not feasible
                       // check if strongest attacker
                       // can take out strongest blocker
                       if (max_attacker_hp>=max_blocker_hp)
                       {
                            computer_attack_pos[0] = strongest_attacker; 
                       }
                  }
             }
        }
    }
    
    // perform actual attack
    if (make_united_attack)
    {
        if (computer_attack_pos[0])
        {
             make_computer_attack(computer_attack_pos[0],0,0,computer_attack_pos1,computer_attack_pos2);
             united_attack(COMPUTER_PLAYER,computer_attack_pos1,computer_attack_pos2,human_block_pos[1]);
             card_battle(COMPUTER_PLAYER,computer_attack_pos[0],human_block_pos[0]);
        }
        else
        {
             make_computer_attack(0,0,0,computer_attack_pos1,computer_attack_pos2);
             united_attack(COMPUTER_PLAYER,computer_attack_pos1,computer_attack_pos2,human_block_pos[1]);
        }
    }
    else
    {
        make_computer_attack(computer_attack_pos[0],computer_attack_pos[1],computer_attack_pos[2],0,0);

        for (int i=0;i<3;i++)
        {
                if (computer_attack_pos[i]) card_battle(COMPUTER_PLAYER,computer_attack_pos[i],human_block_pos[i]);
        }
    }
}

void do_computer_block(int attack_pos1, int attack_pos2, int attack_pos3, int united_attack1, int united_attack2)
{
    // this determines the blocking actions performed by the computer
    // it is not smart --- no strategy is formulated by the computer;
    // computer block action is determined by performing elementary comparisons
    // of character statistics
    
    int block_status[3];
    int strongest_blocker = 0;
    int strongest_blocker_hp = 0;
    int num_blockers = 0;
    int weakest_blocker_hp = 0;
    int weakest_blocker = 0;
    
    // check if there are potential blockers
    for (int i=0;i<3;i++)
    {
        block_status[i] = 0;            // clear block status
        computer_block_pos[i] = 255;    // clear block status
        
        if (can_block(COMPUTER_PLAYER,i))
        {
                num_blockers++;
                
                // determine strongest blocker
                int blocker_hp = get_hp(COMPUTER_PLAYER,i);
                
                if (blocker_hp>strongest_blocker_hp)
                {
                     strongest_blocker_hp = blocker_hp;
                     strongest_blocker = i+1;
                }
                
                if (blocker_hp<weakest_blocker_hp||!weakest_blocker)
                {
                     weakest_blocker = i+1;
                     weakest_blocker_hp = blocker_hp;
                }
        }
    }
    
    printf("computer has %d blockers\n",num_blockers);
    
    if (!num_blockers) return;     // no blockers available

    // check if human player performed a united attack
    if (united_attack1)
    {
        // human player performed a united attack
        // if it is a potential threat (can significantly damage computer player HP, find strongest blocker to block it
        // otherwise check if at least 1 can be destroyed by strongest blocker --- if it can be destroyed, block it
        // otherwise ignore it
        
        int total_hp = get_hp(HUMAN_PLAYER,united_attack1-1)+get_hp(HUMAN_PLAYER,united_attack2-1);
        
        if (total_hp>=(player_hp[COMPUTER_PLAYER]/3))
        {
                computer_block_pos[1] = strongest_blocker;
                block_status[strongest_blocker - 1] = 1;  
                num_blockers --;
        }
        else
        {
                // Not a potential threat to computer HP
                // check if weakest blocker can destroy at least one of it
                // if it can -- then block it otherwise ignore it completely
                if (weakest_blocker_hp>=total_hp||weakest_blocker_hp>=get_hp(HUMAN_PLAYER,united_attack2-1))
                {
                     computer_block_pos[1] = strongest_blocker;
                     block_status[weakest_blocker - 1] = 1;
                     num_blockers--;
                }
        }
        
        // check if there are other attackers
        if (attack_pos1)
        {
                if (!num_blockers) return;
                
                // check if single attacker is a threat
                if (get_hp(HUMAN_PLAYER,attack_pos1-1)>=player_hp[COMPUTER_PLAYER]/3)
                {
                     // check if strongest blocker blocked the united attack
                     // if it didn't block the united attack, block the stray
                     // attacker otherwise look for the next
                     if (!block_status[strongest_blocker-1])
                     {
                          computer_block_pos[0] = strongest_blocker;
                     }
                     else
                     {
                          int next_strongest_blocker = 0;
                          int next_strongest_blocker_hp = 0;
                          
                          for (int i = 0; i<3; i++)
                          {
                               if (can_block(COMPUTER_PLAYER,i)&&(i+1)!=strongest_blocker)
                               {
                                    int next_blocker_hp = get_hp(COMPUTER_PLAYER,i);
                                    if (next_blocker_hp > next_strongest_blocker_hp)
                                    {
                                         next_strongest_blocker = i + 1;
                                         next_strongest_blocker_hp = next_blocker_hp;
                                    }
                               }
                          }
                          
                          if (next_strongest_blocker) computer_block_pos[0] = next_strongest_blocker;
                     }
                }
                else
                {
                     // stray attacker is not a threat to computer HP
                     // check if it can be destroyed. if it can be destroyed
                     // block it
                     
                     // check first if weakest blocker can still block it
                     if (!block_status[weakest_blocker-1])
                     {
                          if (weakest_blocker_hp>=get_hp(HUMAN_PLAYER,attack_pos1-1)) computer_block_pos[0] = weakest_blocker;
                     }
                     else
                     {
                           // look for another blocker
                           int next_blocker = 0;
                           int next_blocker_hp = 0;
                           
                           for (int i=0; i<3; i++)
                           {
                                if (can_block(COMPUTER_PLAYER,i)&&(i+1)!=weakest_blocker&&(i+1)!=strongest_blocker)
                                {
                                     int blocker_hp = get_hp(COMPUTER_PLAYER,i);
                                     
                                     if (blocker_hp >= get_hp(COMPUTER_PLAYER,attack_pos1-1))
                                     {
                                          if (blocker_hp<=next_blocker_hp||!next_blocker)
                                          {
                                               next_blocker = i+1;
                                               next_blocker_hp = blocker_hp;
                                          }
                                     }
                                }
                           }
                           
                           if (next_blocker) computer_block_pos[0] = next_blocker;
                     }
                }
        }
    }
    else
    {
        // human player did not perform a united attack
        int attackers[3];
        int attack_order[3];
        int num_attackers = 0;
        int min_attacker_hp = 0;
        int min_attacker = 0;
        int max_attacker_hp = 0;
        int max_attacker = 0;
                       
        attackers[0] = attack_pos1;
        attackers[1] = attack_pos2;
        attackers[2] = attack_pos3;
        
        for (int j=0; j<3; j++)
        {
             if (attackers[j])
             {
                  num_attackers++;
                  
                  if (get_hp(HUMAN_PLAYER,attackers[j]-1)>max_attacker_hp)
                  {
                       max_attacker_hp = get_hp(HUMAN_PLAYER,attackers[j]-1);
                       max_attacker = j+1;
                  }
                  
                  if (get_hp(HUMAN_PLAYER,attackers[j]-1)<min_attacker_hp || !min_attacker)
                  {
                       min_attacker_hp = get_hp(HUMAN_PLAYER,attackers[j]-1);
                       min_attacker = j+1;
                  }
             }
             
             attack_order[j] = j+1;
        }

        // this sorts the attackers in decreasing hp
        // (see code before end of this code block)
                
        if (num_attackers == 2)
        {
             if (min_attacker<max_attacker)
             {
                  attack_order[min_attacker - 1] = max_attacker;
                  attack_order[max_attacker - 1] = min_attacker;
                  int temp = attackers[min_attacker-1];
                  attackers[min_attacker-1] = attackers[max_attacker-1];
                  attackers[max_attacker-1] = temp;
             }
        }
        
        if (num_attackers == 3)
        {
             if (min_attacker != 3)
             {
                  attack_order[min_attacker - 1] = attack_order[2];
                  attack_order[2] = min_attacker;
                  int temp = attackers[min_attacker-1];
                  attackers[min_attacker-1] = attackers[2];
                  attackers[2] = temp;
             }
             
             if (get_hp(HUMAN_PLAYER,attackers[0]-1)<get_hp(HUMAN_PLAYER,attackers[1]-1))
             {
                  int temp = attack_order[0];
                  
                  attack_order[0] = attack_order[1];
                  attack_order[1] = temp;
                  
                  temp = attackers[0];
                  
                  attackers[0] = attackers[1];
                  attackers[1] = temp;
             }
        }
        
        for (int j = 0; j<3; j++)
        {
                if (!num_blockers) return;

                // if attacker is present
                // try to match a blocker
                if (attackers[j])
                {
                     // check if attacker is threatening to the computer hp
                     if (get_hp(HUMAN_PLAYER,attackers[j]-1)>=player_hp[COMPUTER_PLAYER]/3)
                     {
                          //it is a threat --- find a blocker to match it
                          // check if strongest blocker can still block it
                          if (!block_status[strongest_blocker-1])
                          {
                               block_status[strongest_blocker-1] = 1;
                               computer_block_pos[j] = strongest_blocker;
                               num_blockers --;
                          }
                          else
                          {
                               // try to find next strongest blocker
                               int blocker = 0;
                               int blocker_hp = 0;
                                    
                               for (int i = 0; i<3; i++)
                               {
                                    if (can_block(COMPUTER_PLAYER,i)&&!block_status[i])
                                    {
                                         int next_blocker_hp = get_hp(COMPUTER_PLAYER,i);
                                         if (next_blocker_hp >= blocker_hp)
                                         {
                                              blocker = i +1;
                                              blocker_hp = next_blocker_hp;
                                         }
                                    }
                               }
                                    
                               if (blocker)
                               {
                                    block_status[blocker-1] = 1;
                                    computer_block_pos[j] = blocker;
                                    num_blockers --;
                               }
                          }
                     }
                     else
                     {
                          // attacker is not a threat to computer hp
                          // check if it can be destroyed.
                          // if it can be destroyed find the minimum
                          // hp needed to destroy it
                          int blocker_hp = 0;
                          int blocker = 0;
                          
                          for (int i = 0; i<3; i++)
                          {
                               if (can_block(COMPUTER_PLAYER,i)&&!block_status[i])
                               {
                                    int next_blocker_hp = get_hp(COMPUTER_PLAYER,i);
                                    if (next_blocker_hp >= get_hp(HUMAN_PLAYER,attackers[j]-1))
                                    {
                                         if (next_blocker_hp<=blocker_hp||!blocker)
                                         {
                                              blocker = i+1;
                                              blocker_hp = next_blocker_hp;
                                         }
                                    }
                               }
                          }
                               
                          if (blocker)
                          {
                               computer_block_pos[j] = blocker;
                               block_status[blocker - 1] = 1;
                               num_blockers --;
                          }
                     }
                }
        }
        
        // sort-out the blocking positions in the original order
        for (int i=0; i<3; i++)
        {
             if (attack_order[i]!= i+1)
             {
                  int temp = computer_block_pos[attack_order[i]-1];
                  computer_block_pos[attack_order[i]-1] = computer_block_pos[i];
                  computer_block_pos[i] = temp;
                  temp = attack_order[attack_order[i]-1];
                  attack_order[attack_order[i]-1] = attack_order[i];
                  attack_order[i] = temp;
             }
        }
    }
}

void do_computer_replace_card()
{
// this allows the computer to make a bluff attack
// in order to replace a card in his combat area

    // check if computer has already deployed a character
    if (deploy_status[COMPUTER_PLAYER]) return;
    
    int num_chars = 0;
    int weakest_char = 0;
    int weakest_hp = 0;
    
    for (int i=0; i<3; i++)
    {
         if (combat_area[COMPUTER_PLAYER][i]!=255)
         {
                  num_chars ++;
                  int char_hp = get_hp(COMPUTER_PLAYER,i);
                  
                  if ((char_hp<weakest_hp||!weakest_char)&&can_attack(COMPUTER_PLAYER,i))
                  {
                       weakest_hp = char_hp;
                       weakest_char = i+1;
                  }
         }
    }
    
    int max_hp = 0;
    int max_hp_char = 0;
    
    for (int i = 0; i<MAX_HAND_SIZE; i++)
    {
        int card_type = hand_card_type[COMPUTER_PLAYER][i];

        if (hand_cards[COMPUTER_PLAYER][i]!=255&&card_type!=ACTION_CARD)
        {
                int card_hp = 0;
                
                if (card_type == SNK_CARD)
                {
                     card_hp = SNK_chars_hp[hand_cards[COMPUTER_PLAYER][i]];
                }
                else
                {
                     card_hp = Capcom_chars_hp[hand_cards[COMPUTER_PLAYER][i]];
                }
                
                if (card_hp>max_hp)
                {
                     max_hp = card_hp;
                     max_hp_char = i;
                }
        }
    }
    
    // check if replacement card better than weakest character in
    // the combat area
    
    if (weakest_char&&(max_hp>weakest_hp))
    {
        make_computer_attack(weakest_char,0,0,0,0);
        card_battle(COMPUTER_PLAYER,weakest_char,human_block_pos[0]);
        
        if (combat_area[COMPUTER_PLAYER][weakest_char-1] == 255)
        {
                place_card(weakest_char-1,COMPUTER_PLAYER,max_hp_char);
                do_special_effects(COMPUTER_PLAYER,weakest_char-1);
                printf("Computer deploys ");
                print_combat_area_card(COMPUTER_PLAYER,weakest_char-1);
                printf("\n");
                set_deploy_status(COMPUTER_PLAYER);
        }
    }
}

int use_card(int card)
{
    int backup_available = 0;
    int vacant_slots = 0;

    int player = HUMAN_PLAYER;
    
    if (hand_card_type[player][card]!=ACTION_CARD)
    {
        for (int i=0;i<3;i++)
        {
                if (combat_area[player][i] == 255) vacant_slots++;
        }
        
        backup_available = check_for_backups(player,card);

        if (!vacant_slots&&!backup_available)
        {
                printf("Unable to use card!\n");
        }
        else
        {
                if (deploy_status[player]) return 1;
                
                int chosen_area = 0;

                view_combat_area(HUMAN_PLAYER);
                
                while(!chosen_area)
                {
                     printf("Choose area 1-3:\n");
                     char c = getch();
                     
                     if (c == '1') chosen_area = 1;
                     if (c == '2') chosen_area = 2;
                     if (c == '3') chosen_area = 3;
                }
                
                chosen_area--;
             
                if (combat_area[player][chosen_area]!=255)
                {
                     if (!check_backup(player,card,chosen_area))
                     {
                          printf("Can't backup!\n");
                     }
                     else
                     {
                          do_backup(player,card,chosen_area);
                     }
                }
                else
                {
                     place_card(chosen_area,player,card);
                     do_special_effects(player,chosen_area);
                     set_deploy_status(player);
                     return 1;                                             
                }
        }
    }
    else
    {
        use_special_card(player,card);
    }
    
    return 0;
}

void print_card_info(int player, int card_num)
{
        int card_type = hand_card_type[player][card_num];
        int card = hand_cards[player][card_num];
        
        if (card_type == SNK_CARD)
        {
                printf("SNK CARD # %d HP: %d SP: %d\n",card,SNK_chars_hp[card],SNK_chars_sp[card]);
        }

        if (card_type == CAPCOM_CARD)
        {
                printf("CAPCOM CARD # %d HP: %d SP: %d\n",card,Capcom_chars_hp[card],Capcom_chars_sp[card]);
        }

        if (card_type == ACTION_CARD)
        {
                printf("ACTION CARD # %d\n",card);
        }
}

void print_data(int player, int card_num)
{
    int card_type = hand_card_type[player][card_num];
    int card = hand_cards[player][card_num];

    print_card_info(player,card_num);

    if (card_type == SNK_CARD)
    {
        printf("Backups:\n");
        
        for (int i=0; i<3; i++)
        {
                if (SNK_backups[card*3+i] != 255)
                {
                     if (SNK_backups[card*3+i]>=120)
                     {
                          printf("[CAPCOM #%d]\n",SNK_backups[card*3+i]-120);
                     }
                     else
                     {
                          printf("[SNK #%d]\n",SNK_backups[card*3+i]);
                     }
                }
        }
    }
}

void print_hand(int player)
{
    hand_size[player] = 0;
    
    for (int i=0; i<MAX_HAND_SIZE; i++)
    {
        if (hand_card_type[player][i]!=255)
        {
             hand_size[player]++;
             printf("%2d - ",hand_size[player]);
             print_card_info(player,i);
        }
    }
}

int print_menu(void)
{
    int exit_flag = 0;  
    
    while(!exit_flag)
    {
        printf("\n1 - HAND\n");
        printf("2 - ATTACK\n");
        printf("3 - ABILITY\n");
        printf("4 - INFO\n");
        printf("5 - SEARCH\n");
        printf("6 - END\n");
        
        char c = getch();
        
        if (c == '1') exit_flag = 1;
        if (c == '2') exit_flag = 2;
        if (c == '3') { exit_flag = 3; printf("Feature not implemented yet.\n"); }
        if (c == '4') { exit_flag = 4; printf("Feature not implemented yet.\n"); }
        if (c == '5') { exit_flag = 5; printf("Feature not implemented yet.\n"); }
        if (c == '6') exit_flag = 6;
    }
    
    printf("\n");
    
    return exit_flag;
}

int return_card(int player,int card_num)
{
    int found_card = 0;
    int card_in_hand = 0;
    int i = 0;
    
    while (i<MAX_HAND_SIZE)
    {
        if (hand_cards[player][i]!=255)
        {
                found_card++;
        }
        
        if (found_card == card_num) { card_in_hand = i; break; }
        i++;
    }

    return card_in_hand;
}

int view_hand()
{
    int player = HUMAN_PLAYER;
    
    int return_flag = 0;
    
    while(!return_flag)
    {
        int exit_flag = 0;
        int chosen_card = 0;
        
        while (!exit_flag)
        {
            print_hand(player);
            printf(" 0 - EXIT\n");
            printf("choose card: ");
            scanf("%d",&chosen_card);
            if (chosen_card<0||chosen_card>hand_size[player]) printf("you do not have that card!\n"); else exit_flag = 1;
        }
    
        if (chosen_card == 0) return 1;
    
        int selected_card = return_card(player,chosen_card);

        int backups = check_for_backups(player,selected_card);
        
        if (backups) printf("This card can backup character in %d\n",backups); else printf("This card cannot backup a character in your combat area\n");
            
        exit_flag = 0;
    
        while (!exit_flag)
        {
                printf("\n1 - USE\n");
                printf("2 - DATA\n");
                printf("3 - EXIT\n");    
                char c = getch();
                if (c == '1') exit_flag = 1;
                if (c == '2') exit_flag = 2;
                if (c == '3') exit_flag = 3;
        }
        
        if (exit_flag == 2)
        {
               printf("\n");
               print_data(player,selected_card);
               printf("\n");
               return_flag = 0;
        }
        
        if (exit_flag == 1)
        {
                use_card(selected_card);
                return_flag = 1;
        }
    }
    
    return 0;
}

void do_attack()
{
    int player = HUMAN_PLAYER;
    
    int current_attack_status[3];
    int attack_pos1 = 0;
    int attack_pos2 = 0;
    int make_united_attack = 0;
    
    int num_attackers  = 0;
    
    for (int i=0; i<3; i++)
    {
        num_attackers += can_attack(player,i);
        current_attack_status[i] = 0;
    }
    
    if (!num_attackers) { printf("You do not have any characters that can attack\n"); return; }
    
    printf("You have %d characters that can attack\n",num_attackers);
    
    int attacker_flag = 0;
    int current_num_attackers = 0;
    
    while(!attacker_flag)
    {
        for (int i=0;i<3; i++)
        {
                if (can_block(COMPUTER_PLAYER,i))
                {
                      printf("Computer can block with "); 
                      print_combat_area_card(COMPUTER_PLAYER,i);
                      printf("\n");
                }
        }

        for (int i=0;i<3;i++)
        {
                printf("%d - ",i+1);

                if (combat_area[player][i] == 255)
                {
                     printf("[EMPTY]\n");
                }
                else
                {
                     print_combat_area_card(HUMAN_PLAYER,i);

                     if (can_attack(player,i))
                     {
                          if (current_attack_status[i])
                          {
                               printf(" - (ATTACKING)\n");
                          }
                          else
                          {
                               printf(" - (NOT ATTACKING)\n");
                          }
                     }
                     else
                     {
                          printf(" - (CANNOT ATTACK)\n");
                     }
                }
        }
  
        if (make_united_attack)
        {
                printf("card %d and %d performing united attack\n",attack_pos1,attack_pos2);
        }      
        
        printf("choose attacker (4 - to make attack):\n");
        
        char c = getch();
        
        if (c == '1' || c == '2' || c == '3')
        {
                int attacker_num = c - '0';
                
                if (combat_area[player][attacker_num-1] == 255)
                {
                     printf("Area empty\n");
                }
                else
                {
                     if (can_attack(player,attacker_num-1))
                     {
                          if (current_attack_status[attacker_num-1])
                          {
                               current_attack_status[attacker_num-1] = 0;
                               current_num_attackers--;

                               if (current_num_attackers == 2)
                               {
                                    if (make_united_attack)
                                    {
                                         if (attack_pos1 == attacker_num || attack_pos2 == attacker_num)
                                         {
                                              make_united_attack = 0;
                                              
                                              if (attack_pos1 == attacker_num)
                                              {
                                                   attack_pos1 = attack_pos2;
                                              }
                                              
                                              attack_pos2 = 0;
                                         }
                                    }
                                    else
                                    {
                                         if (attack_pos1 == attacker_num)
                                         {
                                              attack_pos1 = 0;
                                         }
                                    }
                               }

                               if (current_num_attackers == 1)
                               {
                                    if (make_united_attack)
                                    {
                                         if (attack_pos2 != attacker_num) attack_pos1 = attack_pos2;
                                    }
                                    
                                    attack_pos2 = 0;
                                    make_united_attack = 0;
                               }
                          }
                          else
                          {
                               current_attack_status[attacker_num-1] = 1;
                               current_num_attackers++;
                               
                               if (current_num_attackers == 2)
                               {
                                    if (!make_united_attack) attack_pos2 = attacker_num;
                               }
                               else
                               {
                                    if (current_num_attackers == 3)
                                    {
                                         if (!make_united_attack) attack_pos2 = attacker_num;
                                    }
                                    else
                                    {
                                         attack_pos1 = attacker_num;
                                    }
                               }
                          }
                     }
                     else
                     {
                          printf("Character cannot attack\n");
                     }
                }
                
                // check for united attacks
                if (attack_pos1&&attack_pos2&&current_num_attackers>=2&&player_sp[player]>=5&&!make_united_attack&&current_attack_status[attack_pos1-1]&&current_attack_status[attack_pos2-1])
                {
                     printf("1 - UNITED ATTACK\n2 - SINGLE\nChoose attack type:\n");
                     int united_or_single_flag = 0;
                     while (!united_or_single_flag)
                     {
                          char c2 = getch();
                          if (c2 == '1') { make_united_attack = 1; united_or_single_flag = 1; }
                          if (c2 == '2') { united_or_single_flag = 1; }
                     }
                }
        }
        
        if (c == '4') attacker_flag = 1;
    }
    
    // Perform actual attack
    if (!make_united_attack)
    {
        int attack_pos[3];
                
        for (int i=0; i<3; i++)
        {
                if (current_attack_status[i]) attack_pos[i] = i+1; else attack_pos[i] = 0;
        }

        do_computer_block(attack_pos[0],attack_pos[1],attack_pos[2],0,0);
                
        for (int i=0; i<3; i++)
        {
                if (attack_pos[i])
                {
                     if (computer_block_pos[i]&&computer_block_pos[i]!=255)
                     {
                          printf("Computer blocks ");
                          print_combat_area_card(HUMAN_PLAYER,attack_pos[i]-1);
                          printf(" with ");
                          print_combat_area_card(COMPUTER_PLAYER,computer_block_pos[i]-1);
                          printf("\n");
                     }
                     
                     card_battle(player,attack_pos[i],computer_block_pos[i]);
                }
        }
    }
    else
    {
        if (current_num_attackers == 3)
        {
                int single_attacker = 0;
                     
                for (int i=0;i<3;i++)
                {
                     if (i!=attack_pos1&&i!=attack_pos2) single_attacker = i;
                }
                     
                do_computer_block(single_attacker,0,0,attack_pos1,attack_pos2);
                
                if (computer_block_pos[1]&&computer_block_pos[1]!=255)
                {
                     printf("Computer blocks ");
                     print_combat_area_card(HUMAN_PLAYER,attack_pos1-1);
                     printf(" and ");
                     print_combat_area_card(HUMAN_PLAYER,attack_pos2-1);
                     printf(" with ");
                     print_combat_area_card(COMPUTER_PLAYER,computer_block_pos[1]-1);
                }
                
                united_attack(player,attack_pos1,attack_pos2,computer_block_pos[1]);

                if (computer_block_pos[0]&&computer_block_pos[0]!=255)
                {
                     printf("Computer blocks ");
                     print_combat_area_card(HUMAN_PLAYER,single_attacker-1);
                     printf(" with ");
                     print_combat_area_card(COMPUTER_PLAYER,computer_block_pos[0]-1);
                     
                }
                
                card_battle(player,single_attacker,computer_block_pos[0]);
        }
        else
        {
                do_computer_block(0,0,0,attack_pos1,attack_pos2);

                if (computer_block_pos[1]&&computer_block_pos[1]!=255)
                {
                     printf("Computer blocks ");
                     print_combat_area_card(HUMAN_PLAYER,attack_pos1-1);
                     printf(" and ");
                     print_combat_area_card(HUMAN_PLAYER,attack_pos2-1);
                     printf(" with ");
                     print_combat_area_card(COMPUTER_PLAYER,computer_block_pos[1]-1);
                     printf("\n");
                }

                united_attack(player,attack_pos1,attack_pos2,computer_block_pos[1]);
        }
    }
}

int main(void)
{
    int current_player = HUMAN_PLAYER;
    player_hp[HUMAN_PLAYER] = 1000;
    player_hp[COMPUTER_PLAYER] = 1000;
    player_sp[HUMAN_PLAYER] = 0;
    player_sp[COMPUTER_PLAYER] = 0;
    
    printf("SVC_V2 Card Battle Engine Simulator v0.0\n\n");
    
    clear_combat_area();
    
    build_random_deck(HUMAN_PLAYER);
    build_random_deck(COMPUTER_PLAYER);
    init_hand(HUMAN_PLAYER);
    init_hand(COMPUTER_PLAYER);
    init_discard_pile(HUMAN_PLAYER);
    init_discard_pile(COMPUTER_PLAYER);
    
    while (player_hp[HUMAN_PLAYER]&&player_hp[COMPUTER_PLAYER])
    {
        draw_card(current_player);
        clear_deploy_status(current_player);
        clear_attack_status(current_player);
        clear_freeze_status(current_player);
        clear_backup_status(current_player);
        
        make_combat_ready(current_player);
        
        if (current_player == HUMAN_PLAYER)
        {
                int choice = 0;
                
                while (choice != 6)
                {
                     view_combat_area(1-current_player);
                     view_combat_area(current_player);
                     
                     choice = print_menu();
                     if (choice == 1) view_hand();
                     if (choice == 2)
                     {
                          do_attack();
                          if (!player_hp[COMPUTER_PLAYER]) choice = 6;
                     }
                }
        }
        else
        {
                do_computer_view_hand();
                
                int human_hp = player_hp[HUMAN_PLAYER];
                
                do_computer_attack();
                
                if (player_hp[HUMAN_PLAYER]<human_hp) view_combat_area(1-current_player);
                if (player_hp[HUMAN_PLAYER]) do_computer_view_hand();
                if (!deploy_status[COMPUTER_PLAYER]) do_computer_replace_card();
        }
        
        
        if (player_hp[COMPUTER_PLAYER]&&player_hp[HUMAN_PLAYER])
        {
                do_end_of_turn_effects(current_player);
                current_player = 1 - current_player;
                if (current_player == COMPUTER_PLAYER) printf("\nIt is now the computer's turn!\n"); else printf("\nIt is now your turn!\n");
        }
    }

    if (!player_hp[COMPUTER_PLAYER]) printf("You win!\n"); else printf("Computer wins!\n");
    system("PAUSE");
    return 0;
}

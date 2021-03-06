#include "../include/actions.h"
#include "../include/object.h"
#include "../include/board.h"
#include "../include/projectiles.h"
#include "action_classes.h"
#include <iostream>
#include <fstream>

int give_mana(std::string name){
    std::ifstream in("/opt/Magic_fight/configs/mana.cfg");
    std::string tmp_name;
    int tmp_mana;
    do {
        in >> tmp_name >> tmp_mana;
    } while (tmp_name!=name && tmp_name!="");

    in.close();
    if (tmp_name==""){
        return 1000000;
    }
    return tmp_mana;
}

action::action(hero* owner){
    this->owner=owner;
}


make_step::make_step(hero* owner):action(owner){
    this->mana_costs=give_mana("make_step");
}

void make_step::DoIt(int direction)
{
    if (this->owner->cur_mana<this->mana_costs) return;
    hexagon* destination = owner->position->NextHex(direction);
    if (destination!=nullptr){
        this->owner->cur_mana-=this->mana_costs;
        this->owner->position->owner->ChangeHash();
        this->owner->position->hex_objects.remove(owner);
        this->owner->position=destination;
        destination->hex_objects.push_back(owner);
    }
}

make_leap::make_leap(hero* owner):action(owner){
    this->mana_costs=give_mana("make_leap");
}

void make_leap::DoIt(int direction)
{
    if (this->owner->cur_mana<this->mana_costs) return;
    hexagon* destination = owner->position->NextHex(direction);
    if (destination!=nullptr){
        this->owner->cur_mana-=this->mana_costs;
        this->owner->position->owner->ChangeHash();
        this->owner->position->hex_objects.remove(owner);
        this->owner->position=destination;
        destination->hex_objects.push_back(owner);
    }
    destination = owner->position->NextHex(direction);
    if (destination!=nullptr){
        this->owner->position->hex_objects.remove(owner);
        this->owner->position=destination;
        destination->hex_objects.push_back(owner);
    }
}

make_fireball::make_fireball(hero* owner):action(owner){
    this->mana_costs=give_mana("make_fireball");
}

void make_fireball::DoIt(int direction)
{
    if (this->owner->cur_mana<this->mana_costs) return;
    this->owner->cur_mana-=this->mana_costs;
    this->owner->position->owner->ChangeHash();
    new fireball(this->owner->position->NextHex(direction), this->owner->number, direction);
}

passive_action::passive_action(hero* owner):action(owner){}

void passive_action::DoIt(int direction){}

class_wizard::class_wizard(hero* owner):passive_action(owner){
    new mana_creator(&this->owner->position->owner->event_queue, 1, owner, 1000);
}

void class_wizard::DoIt(int direction){}

potion::potion(hero* owner, int amount):action(owner){
    this->amount=amount;
}

void potion::DoIt(int direction){}

smoke_grenade::smoke_grenade(hero* owner, int amount):potion(owner, amount){}

void smoke_grenade::DoIt(int diretion){}

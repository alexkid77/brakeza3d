//
// Created by darkhead on 2/1/20.
//

#include "../../headers/Game/ItemHealthGhost.h"

void ItemHealthGhost::setAid(float aid) {
    ItemHealthGhost::aid = aid;
}

ItemHealthGhost::ItemHealthGhost() : aid(25) {

}

float ItemHealthGhost::getAid() const {
    return aid;
}
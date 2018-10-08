/* $Id: commands.cpp,v 1.1.2.8 2010-09-01 21:20:44 rufina Exp $ 
 *
 * ruffina, 2004
 */
/***************************************************************************
 * Все права на этот код 'Dream Land' пренадлежат Igor {Leo} и Olga {Varda}*
 * Некоторую помощь в написании этого кода, а также своими идеями помогали:*
 *    Igor S. Petrenko     {NoFate, Demogorgon}                            *
 *    Koval Nazar          {Nazar, Redrum}                                 *
 *    Doropey Vladimir     {Reorx}                                         *
 *    Kulgeyko Denis       {Burzum}                                        *
 *    Andreyanov Aleksandr {Manwe}                                         *
 *    и все остальные, кто советовал и играл в этот MUD                    *
 ***************************************************************************/

#include "commandtemplate.h"

#include "pcharacter.h"
#include "npcharacter.h"
#include "room.h"
#include "clanreference.h"

#include "follow_utils.h"
#include "loadsave.h"
#include "mercdb.h"
#include "act.h"
#include "merc.h"
#include "def.h"

CLAN(battlerager);
Character * follower_find_nosee( Character *ch, const char *cargument );

static bool check_mutual_induct( Character *ch, Character *victim, ClanReference &clan )
{
    if (ch->is_npc( ))
        return true;
    
    bool isClanMember = (ch->getClan( ) == clan);
    bool isNotAllowed = (!isClanMember && !clan->canInduct( ch->getPC( ) ));

    for (Character *gch = char_list; gch; gch = gch->next) {
        if (gch->is_npc( ))
            continue;

        if (!is_same_group( gch, victim ))
            continue;

        if (isClanMember && !clan->canInduct( gch->getPC( ) ))
            return false;

        if (isNotAllowed && gch->getClan( ) == clan)
            return false;
    }

    return true;
}

CMDRUN( follow )
{
/* RT changed to allow unlimited following and follow the NOFOLLOW rules */
    Character *victim;
    DLString arg;
    DLString arguments = constArguments;;
    
    arg = arguments.getOneArgument( );

    if (arg.empty( )) {
        ch->send_to( "Следовать за кем?\n\r");
        return;
    }

    if ( ( victim = get_char_room( ch, arg.c_str( ) ) ) == 0 ) {
        ch->send_to( "Ты не находишь этого здесь.\n\r");
        return;
    }

    if ( IS_AFFECTED(ch, AFF_CHARM) && ch->master != 0 ) {
        act_p( "Но тебе хочется следовать за $C5!", ch, 0, ch->master, TO_CHAR,POS_RESTING );
        return;
    }

    if (victim == ch)
    {
        if ( ch->master == 0 )
        {
            ch->send_to( "Ты уже следуешь за собой.\n\r");
            return;
        }
        ch->stop_follower();
        return;
    }
    
    if (!check_mutual_induct( ch, victim, clan_battlerager )) {
        act("Ты не сможешь следовать за $C5.", ch, 0, victim, TO_CHAR);
        return;
    }

    if( !victim->is_npc() &&
        IS_SET( victim->act, PLR_NOFOLLOW ) &&
        !ch->is_immortal() ) 
    {
        act_p("$C1 не желает ходить с кем-либо.\n\r",
             ch,0,victim, TO_CHAR,POS_RESTING);
        return;
    }

    REMOVE_BIT(ch->act,PLR_NOFOLLOW);

    if (ch->master != 0)
        ch->stop_follower();

    ch->add_follower( victim );
}

CMDRUN( group )
{
    Character *victim;
    DLString argument = constArguments;
    DLString arg = argument.getOneArgument( );

    if (arg.empty( ))
    {
        Character *leader;

        leader = (ch->leader != 0) ? ch->leader : ch;
        ch->printf( "Группа %s:\n\r", ch->sees(leader,'2').c_str( ) );

        for (Character *gch = char_list; gch != 0; gch = gch->next )
            if (is_same_group( gch, ch )) {
                if (gch->is_npc( ))
                    ch->pecho( "[%3d    ] %-16.16s{x %5d/%-5d hp %5d/%-5d mana %4d/%-4d mv",
                        gch->getRealLevel( ),
                        ch->sees( gch, '1' ).c_str( ),
                        gch->hit.getValue( ), gch->max_hit.getValue( ), 
                        gch->mana.getValue( ), gch->max_mana.getValue( ),
                        gch->move.getValue( ), gch->max_move.getValue( ) );
                else
                    ch->pecho( "[%3d %3s] %-16.16s %5d/%-5d hp %5d/%-5d mana %4d/%-4d mv %5d xp",
                        gch->getRealLevel( ), 
                        gch->getProfession( )->getWhoNameFor( ch ).c_str( ),
                        ch->sees( gch, '1' ).c_str( ),
                        gch->hit.getValue( ), gch->max_hit.getValue( ), 
                        gch->mana.getValue( ), gch->max_mana.getValue( ),
                        gch->move.getValue( ), gch->max_move.getValue( ),
                        gch->getPC( )->getExpToLevel( ) );
            }

        return;
    }

    if ( ( victim = get_char_room( ch, arg.c_str( ) ) ) == 0 )
    {
        ch->send_to( "Этого нет здесь.\n\r");
        return;
    }

    if (victim == ch) {
        ch->println( "А смысл?" );
        return;
    }

    if ( ch->master != 0 || ( ch->leader != 0 && ch->leader != ch ) )
    {
        ch->send_to( "Но ты следуешь за кем-то еще!\n\r");
        return;
    }

    if (victim->master != ch) {
        act_p( "$C1 не следует за тобой.", ch, 0, victim, TO_CHAR,POS_RESTING );
        return;
    }

    if (IS_AFFECTED(victim,AFF_CHARM)) {
        ch->send_to("Ты не можешь исключить очарованных монстров из своей группы.\n\r");
        return;
    }

    if (IS_AFFECTED(ch,AFF_CHARM)) {
        act_p("Ты любишь своего мастера так сильно, что не можешь покинуть $s!",ch,0,victim,TO_VICT,POS_RESTING);
        return;
    }


    if (is_same_group( victim, ch )) {        
        guarding_nuke( ch, victim );

        victim->leader = 0;
        act_p( "$c1 исключает $C4 из $s группы.",ch,0,victim,TO_NOTVICT,POS_RESTING);
        act_p( "$c1 исключает тебя из $s группы.",ch,0,victim,TO_VICT,POS_SLEEPING);
        act_p( "Ты исключаешь $C4 из своей группы.",ch,0,victim,TO_CHAR,POS_SLEEPING);
        
        guarding_assert( victim );
        return;
    }

    if ( abs(ch->getModifyLevel() - victim->getModifyLevel()) > 8)
    {
        act_p( "$C1 не может присоединиться к группе $c2.",
                ch,0,victim,TO_NOTVICT,POS_RESTING );
        act_p( "Ты не можешь присоединиться к группе $c2.",
                ch,0,victim,TO_VICT,POS_SLEEPING );
        act_p( "$C1 не может присоединиться к твоей группе.",
                ch,0,victim,TO_CHAR,POS_SLEEPING );
        return;
    }

    if (IS_GOOD(ch)
        && IS_EVIL(victim)
        && ( ch->getClan() != victim->getClan()
             || ch->getClan( )->isDispersed( ) ))
    {
        act_p("Ты слишком пло$Gхое|хой|хая для группы $c2.", ch, 0, victim,
                TO_VICT,POS_SLEEPING);
        act_p("$C1 слишком пло$Gхое|хой|хая для твоей группы!", ch, 0, victim,
                TO_CHAR,POS_SLEEPING);
        return;
    }

    if ( IS_GOOD(victim)
            && IS_EVIL(ch)
            && ( ch->getClan() != victim->getClan()
                    || ch->getClan()->isDispersed( ) ) )
    {
        act_p("Ты слишком хоро$Gшее|ший|шая для группы $c2!", ch, 0, victim,
                TO_VICT,POS_SLEEPING);
        act_p("$C1 слишком хоро$Gшее|ший|шая для твоей группы!", ch, 0, victim,
                TO_CHAR,POS_SLEEPING);
        return;
    }

    if (!victim->is_npc( ) 
        && (ch->getClan( )->isEnemy( *victim->getClan( ) )
            || victim->getClan( )->isEnemy( *ch->getClan( ) )))
    {
        act_p("Ты же ненавидишь клан $c2, как ты можешь присоединиться к $s группе?!", ch,
                0, victim,TO_VICT,POS_SLEEPING);
        act_p("Ты же ненавидишь клан $C2, как ты можешь предлагать $M присоединиться к твоей группе?!",
                ch, 0, victim, TO_CHAR,POS_SLEEPING);
        return;
    }

    if (!check_mutual_induct( ch, victim, clan_battlerager )) {
        act_p("Ты не сможешь вступить в группу $C2.", ch, 0, victim, TO_VICT, POS_SLEEPING);
        act("$C1 не сможет вступить в твою группу.", ch, 0, victim, TO_CHAR);
        return;
    }

    victim->leader = ch;
    act_p( "$C1 присоединил$Gось|ся|ась к группе $c2.", ch, 0, victim,TO_NOTVICT, POS_RESTING);
    act_p( "Ты присоединил$Gось|ся|ась к группе $c2.", ch, 0, victim,TO_VICT, POS_SLEEPING);
    act_p( "$C1 присоединил$Gось|ся|ась к твоей группе.", ch, 0, victim, TO_CHAR, POS_SLEEPING);
}


CMDRUN( nuke )
{
    Character *victim;
    DLString argument = constArguments;
    DLString arg = argument.getOneArgument( );

    if (arg.empty( )) {
        ch->println( "Чье следование за тобой ты хочешь прекратить?" );
        return;
    }
    
    if ( !( victim = follower_find_nosee( ch, arg.c_str( ) ) )) {
        ch->println( "Среди твоих последователей нет никого с таким именем." );
        return;
    }

    if (ch == victim) {
        ch->println( "От себя не убежишь." );
        return;
    }
    
    if (is_same_group( victim, ch )) {
        guarding_nuke( ch, victim );
        victim->leader = 0;
        guarding_assert( victim );
    }
    
    act( "$c1 исключает $C4 из числа $s последователей.",ch,0,victim,TO_NOTVICT);
    act_p( "$c1 исключает тебя из числа $s последователей.",ch,0,victim,TO_VICT,POS_SLEEPING);
    act_p( "Ты исключаешь $C4 из числа твоих последователей.",ch,0,victim,TO_CHAR,POS_SLEEPING);
    victim->stop_follower( );
}


/*
 * 'Split' originally by Gnort, God of Chaos.
 */
CMDRUN( split )
{
    char buf[MAX_STRING_LENGTH];
    DLString arg1, arg2;
    Character *gch;
    int members;
    int amount_gold = 0, amount_silver = 0;
    int share_gold, share_silver;
    int extra_gold, extra_silver;
    DLString argument = constArguments;
    
    arg1 = argument.getOneArgument( );
    arg2 = argument.getOneArgument( );

    if (arg1.empty( ))
    {
        ch->send_to( "Разделить? Сколько?\n\r");
        return;
    }

    amount_silver = atoi( arg1.c_str() );

    if (!arg2.empty())
        amount_gold = atoi(arg2.c_str());

    if ( amount_gold < 0 || amount_silver < 0)
    {
        ch->send_to( "Твоей группе это не понравится.\n\r");
        return;
    }

    if ( amount_gold == 0 && amount_silver == 0 )
    {
        ch->send_to( "Ты не взял ни одной монеты, но никому об этом не сказал.\n\r");
        return;
    }

    if ( ch->gold <  amount_gold || ch->silver < amount_silver)
    {
        ch->send_to( "У тебя нет столько, чтоб поделиться.\n\r");
        return;
    }

    members = 0;
    for ( gch = ch->in_room->people; gch != 0; gch = gch->next_in_room )
    {
        if ( is_same_group( gch, ch ) && !IS_AFFECTED(gch,AFF_CHARM))
            members++;
    }

    if ( members < 2 )
    {
        ch->send_to( "Можешь забрать себе все.\n\r");
        return;
    }
        
    share_silver = amount_silver / members;
    extra_silver = amount_silver % members;

    share_gold   = amount_gold / members;
    extra_gold   = amount_gold % members;

    if ( share_gold == 0 && share_silver == 0 )
    {
         ch->send_to( "Очень мудро.\n\r");
        return;
    }

    ch->silver        -= amount_silver;
    ch->silver        += share_silver + extra_silver;
    ch->gold         -= amount_gold;
    ch->gold         += share_gold + extra_gold;

    if (share_silver > 0)
    {
        sprintf(buf,
            "Ты делишь %d серебрянн%s. Ты получаешь %d серебра.\n\r",
             amount_silver,GET_COUNT(amount_silver,"ую монету","ые монеты","ых монет"),
            share_silver + extra_silver);
        ch->send_to(buf);
    }

    if (share_gold > 0)
    {
        sprintf(buf,
            "Ты делишь %d золот%s. Ты получаешь %d золота.\n\r",
             amount_gold,GET_COUNT(amount_silver,"ую монету","ые монеты","ых монет"),
             share_gold + extra_gold);
        ch->send_to(buf);
    }

    if (share_gold == 0)
    {
        sprintf(buf,"$c1 делит %d серебрянн%s. Ты получаешь %d серебра.",
                amount_silver,GET_COUNT(amount_silver,"ую монету","ые монеты","ых монет"),
                share_silver);
    }
    else if (share_silver == 0)
    {
        sprintf(buf,"$c1 делит %d золот%s. Ты получаешь %d золота.",
                amount_gold,GET_COUNT(amount_silver,"ую монету","ые монеты","ых монет"),
                share_gold);
    }
    else
    {
        sprintf(buf,"$c1 делит %d серебра и %d золота, дает тебе %d серебра и %d золота.\n\r",
         amount_silver,amount_gold,share_silver,share_gold);
    }

    for ( gch = ch->in_room->people; gch != 0; gch = gch->next_in_room )
    {
        if ( gch != ch && is_same_group(gch,ch) && !IS_AFFECTED(gch,AFF_CHARM))
        {
            act_p( buf, ch, 0, gch, TO_VICT,POS_RESTING );
            gch->gold += share_gold;
            gch->silver += share_silver;
        }
    }

}

CMDRUN( nofollow )
{
    if (ch->is_npc())
        return;

    if ( IS_AFFECTED( ch, AFF_CHARM ) )  {
        ch->send_to( "Ты не можешь покинуть своего повелителя.\n\r");
        return;
    }

    if (IS_SET(ch->act,PLR_NOFOLLOW))
    {
      ch->send_to("Теперь ты разрешаешь следовать за собой.\n\r");
      REMOVE_BIT(ch->act,PLR_NOFOLLOW);
    }
    else
    {
      ch->send_to("Теперь ты не разрешаешь следовать за собой.\n\r");
      SET_BIT(ch->act,PLR_NOFOLLOW);
      ch->die_follower( );
    }
}


/* Copyright (C) 2006 - 2012 ScriptDev2 <http://www.scriptdev2.com/>
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 */

/* ScriptData
SDName: Boss Pathaleon the Calculator
SD%Complete: 85
SDComment: The Nether Wraith script needs additional research and update
SDCategory: Tempest Keep, The Mechanar
EndScriptData */

#include "precompiled.h"
#include "mechanar.h"

enum
{
    SAY_AGGRO                       = -1554020,
    SAY_DOMINATION_1                = -1554021,
    SAY_DOMINATION_2                = -1554022,
    SAY_SUMMON                      = -1554023,
    SAY_ENRAGE                      = -1554024,
    SAY_SLAY_1                      = -1554025,
    SAY_SLAY_2                      = -1554026,
    SAY_DEATH                       = -1554027,

    // Spells to be casted
    SPELL_MANA_TAP                  = 36021,
    SPELL_ARCANE_TORRENT            = 36022,
    SPELL_DOMINATION                = 35280,
    SPELL_ARCANE_EXPLOSION_H        = 15453,
    SPELL_FRENZY                    = 36992,
    SPELL_DISGRUNTLED_ANGER         = 35289,        // research where to use this - possible to be used by Nether Wraits when boss is at 20% HP

    //SPELL_SUMMON_NETHER_WRAITH_1    = 35285,
    //SPELL_SUMMON_NETHER_WRAITH_2    = 35286,
    //SPELL_SUMMON_NETHER_WRAITH_3    = 35287,
    //SPELL_SUMMON_NETHER_WRAITH_4    = 35288,

    // Add Spells
    SPELL_DETONATION                = 35058,
    SPELL_ARCANE_BOLT               = 20720,

    NPC_NETHER_WRAITH               = 21062
};

//static const uint32 aWraithSummonSpells[4] = {SPELL_SUMMON_NETHER_WRAITH_1, SPELL_SUMMON_NETHER_WRAITH_2, SPELL_SUMMON_NETHER_WRAITH_3, SPELL_SUMMON_NETHER_WRAITH_4};

struct MANGOS_DLL_DECL boss_pathaleon_the_calculatorAI : public ScriptedAI
{
    boss_pathaleon_the_calculatorAI(Creature* pCreature) : ScriptedAI(pCreature)
    {
        m_pInstance = (ScriptedInstance*)pCreature->GetInstanceData();
        m_bIsRegularMode = pCreature->GetMap()->IsRegularDifficulty();
        Reset();
    }

    ScriptedInstance* m_pInstance;
    bool m_bIsRegularMode;

    uint32 m_uiSummonTimer;
    uint32 m_uiManaTapTimer;
    uint32 m_uiArcaneTorrentTimer;
    uint32 m_uiDominationTimer;
    uint32 m_uiArcaneExplosionTimer;

    bool m_bIsEnraged;

    GUIDList m_lNWraithGUIDList;

    void Reset()
    {
        m_uiSummonTimer          = 30000;
        m_uiManaTapTimer         = urand(12000, 20000);
        m_uiArcaneTorrentTimer   = urand(16000, 25000);
        m_uiDominationTimer      = urand(25000, 40000);
        m_uiArcaneExplosionTimer = urand(8000, 13000);
        m_bIsEnraged             = false;
    }

    void Aggro(Unit* pWho)
    {
        DoScriptText(SAY_AGGRO, m_creature);
    }

    void KilledUnit(Unit* pVictim)
    {
        DoScriptText(urand(0, 1) ? SAY_SLAY_1 : SAY_SLAY_2, m_creature);
    }

    void JustDied(Unit* pKiller)
    {
        DoScriptText(SAY_DEATH, m_creature);

        if (m_pInstance)
            m_pInstance->SetData(TYPE_PATHALEON, DONE);
    }

    void JustSummoned(Creature* pSummoned)
    {
        m_lNWraithGUIDList.push_back(pSummoned->GetObjectGuid());
        if (m_creature->getVictim())
            pSummoned->AI()->AttackStart(m_creature->getVictim());
    }

    void UpdateAI(const uint32 uiDiff)
    {
        //Return since we have no target
        if (!m_creature->SelectHostileTarget() || !m_creature->getVictim())
            return;

        if (m_uiSummonTimer < uiDiff && m_creature->GetHealthPercent() > 21.0f)
        {
            uint8 uiMaxWraith = urand(3, 4);
            for (uint8 i = 0; i < uiMaxWraith; ++i)
                m_creature->SummonCreature(NPC_NETHER_WRAITH, 0.0f, 0.0f, 0.0f, 0.0f, TEMPSUMMON_DEAD_DESPAWN, 0);

            DoScriptText(SAY_SUMMON, m_creature);
            m_uiSummonTimer = urand(30000, 45000);
        }
        else
            m_uiSummonTimer -= uiDiff;

        if (m_uiManaTapTimer < uiDiff)
        {
            if (Unit* pTarget = m_creature->SelectAttackingTarget(ATTACKING_TARGET_RANDOM, 0, SPELL_MANA_TAP, SELECT_FLAG_POWER_MANA))
            {
                if (DoCastSpellIfCan(pTarget, SPELL_MANA_TAP) == CAST_OK)
                    m_uiManaTapTimer = urand(14000, 22000);
            }
        }
        else
            m_uiManaTapTimer -= uiDiff;

        if (m_uiArcaneTorrentTimer < uiDiff)
        {
            if (DoCastSpellIfCan(m_creature, SPELL_ARCANE_TORRENT) == CAST_OK)
                m_uiArcaneTorrentTimer = urand(12000, 18000);
        }
        else
            m_uiArcaneTorrentTimer -= uiDiff;

        if (m_uiDominationTimer < uiDiff)
        {
            if (Unit* pTarget = m_creature->SelectAttackingTarget(ATTACKING_TARGET_RANDOM, 1))
            {
                if (DoCastSpellIfCan(pTarget, SPELL_DOMINATION) == CAST_OK)
                {
                    DoScriptText(urand(0, 1) ? SAY_DOMINATION_1 : SAY_DOMINATION_2, m_creature);
                    m_uiDominationTimer = urand(25000, 30000);
                }
            }
        }
        else
            m_uiDominationTimer -= uiDiff;

        // Only casting if Heroic Mode is used
        if (!m_bIsRegularMode)
        {
            if (m_uiArcaneExplosionTimer < uiDiff)
            {
                if (DoCastSpellIfCan(m_creature, SPELL_ARCANE_EXPLOSION_H) == CAST_OK)
                    m_uiArcaneExplosionTimer = urand(10000, 14000);
            }
            else
                m_uiArcaneExplosionTimer -= uiDiff;
        }

        if (!m_bIsEnraged && m_creature->GetHealthPercent() < 21.0f)
        {
            if (DoCastSpellIfCan(m_creature, SPELL_FRENZY) == CAST_OK)
            {
		        for (GUIDList::const_iterator itr = m_lNWraithGUIDList.begin(); itr != m_lNWraithGUIDList.end(); ++itr)
                {
                    if (Creature* pCreature = m_creature->GetMap()->GetCreature(*itr))
                    {
                        if (pCreature->isAlive())
                        pCreature->ForcedDespawn();
                    }
                }
                m_lNWraithGUIDList.clear();

                DoScriptText(SAY_ENRAGE, m_creature);
                m_bIsEnraged = true;
            }
        }

        DoMeleeAttackIfReady();
    }
};

struct MANGOS_DLL_DECL mob_nether_wraithAI : public ScriptedAI
{
    mob_nether_wraithAI(Creature* pCreature) : ScriptedAI(pCreature) {Reset();}

    uint32 m_uiArcaneMissilesTimer;
    uint32 m_uiDetonationTimer;

    void Reset()
    {
        m_uiArcaneMissilesTimer = urand(1000, 4000);
        m_uiDetonationTimer = 20000;
    }

    void UpdateAI(const uint32 uiDiff)
    {
        if (!m_creature->SelectHostileTarget() || !m_creature->getVictim())
            return;

        if (m_uiArcaneMissilesTimer < uiDiff)
        {
            Unit* pTarget = m_creature->SelectAttackingTarget(ATTACKING_TARGET_RANDOM, 1);
            if (!pTarget)
                pTarget = m_creature->getVictim();

            if (pTarget)
            {
                if (DoCastSpellIfCan(pTarget, SPELL_ARCANE_BOLT) == CAST_OK)
                    m_uiArcaneMissilesTimer = urand(5000, 10000);
            }
        }
        else
            m_uiArcaneMissilesTimer -=uiDiff;

        if (m_uiDetonationTimer)
        {
            if (m_uiDetonationTimer <= uiDiff)
            {
                if (DoCastSpellIfCan(m_creature, SPELL_DETONATION) == CAST_OK)
                {
                    // Not sure if this one needs to be despawned here. Needs further research
                    //m_creature->ForcedDespawn(2200);
                    m_uiDetonationTimer = 0;
                }
            }
            else
                m_uiDetonationTimer -= uiDiff;
        }

        DoMeleeAttackIfReady();
    }
};

CreatureAI* GetAI_boss_pathaleon_the_calculator(Creature* pCreature)
{
    return new boss_pathaleon_the_calculatorAI(pCreature);
}

CreatureAI* GetAI_mob_nether_wraith(Creature* pCreature)
{
    return new mob_nether_wraithAI(pCreature);
}

void AddSC_boss_pathaleon_the_calculator()
{
    Script* pNewScript;

    pNewScript = new Script;
    pNewScript->Name = "boss_pathaleon_the_calculator";
    pNewScript->GetAI = &GetAI_boss_pathaleon_the_calculator;
    pNewScript->RegisterSelf();

    pNewScript = new Script;
    pNewScript->Name = "mob_nether_wraith";
    pNewScript->GetAI = &GetAI_mob_nether_wraith;
    pNewScript->RegisterSelf();
}

/* Copyright (C) 2006 - 2011 ScriptDev2 <http://www.scriptdev2.com/>
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
SDName: boss_kalecgos
SD%Complete: 90
SDComment: Timers
SDCategory: Sunwell Plateau
EndScriptData */

#include "precompiled.h"
#include "sunwell_plateau.h"

enum
{
    //kalecgos dragon form
    SAY_EVIL_AGGRO                  = -1580000,
    SAY_EVIL_SPELL_1                = -1580001,
    SAY_EVIL_SPELL_2                = -1580002,
    SAY_EVIL_SLAY_1                 = -1580003,
    SAY_EVIL_SLAY_2                 = -1580004,
    SAY_EVIL_ENRAGE                 = -1580005,

    //kalecgos humanoid form
    SAY_GOOD_AGGRO                  = -1580006,
    SAY_GOOD_NEAR_DEATH_20          = -1580007,
    SAY_GOOD_NEAR_DEATH_10          = -1580008,
    SAY_GOOD_PLRWIN                 = -1580009,

    SAY_SATH_AGGRO                  = -1580010,
    SAY_SATH_DEATH                  = -1580011,
    SAY_SATH_SPELL_1                = -1580012,
    SAY_SATH_SPELL_2                = -1580013,
    SAY_SATH_SLAY_1                 = -1580014,
    SAY_SATH_SLAY_2                 = -1580015,
    SAY_SATH_ENRAGE                 = -1580016,

    //Kalecgos
    SPELL_SPECTRAL_BLAST            = 44869,
    SPELL_SPECTRAL_REALM_EJECT      = 44845,        // pull the players ouside the spectral realm in case of wipe

    SPELL_ARCANE_BUFFET             = 45018,
    SPELL_FROST_BREATH              = 44799,
    SPELL_TAIL_LASH                 = 45122,
    SPELL_CRAZED_RAGE               = 44807,

    // Kalecgos human
    SPELL_HEROIC_STRIKE             = 45026,
    SPELL_REVITALIZE                = 45027,

     //Sathrovarr
    SPELL_SPECTRAL_INVISIBILITY     = 44801,
    SPELL_CORRUPTING_STRIKE         = 45029,
    SPELL_CURSE_OF_BOUNDLESS_AGONY  = 45032,
    SPELL_SHADOW_BOLT_VOLLEY        = 45031,

    //Misc
    SPELL_BANISH                    = 44836
};

static const uint32 aWildMagicSpells[6]= {44978, 45001, 45002, 45004, 45006, 45010};

struct MANGOS_DLL_DECL boss_kalecgosAI : public ScriptedAI
{
    boss_kalecgosAI(Creature* pCreature) : ScriptedAI(pCreature)
    {
        m_pInstance = (instance_sunwell_plateau*)pCreature->GetInstanceData();
        Reset();
    }

    instance_sunwell_plateau* m_pInstance;

    uint32 m_uiArcaneBuffetTimer;
    uint32 m_uiFrostBreathTimer;
    uint32 m_uiWildMagicTimer;
    uint32 m_uiSpectralBlastTimer;
	uint32 m_uiTailLashTimer;
    uint32 m_uiExitTimer;

    bool m_bIsUncorrupted;
    bool m_bIsBanished;
    bool m_bIsEnraged;

    void Reset()
    {
        m_uiArcaneBuffetTimer       = 8000;
		m_uiTailLashTimer           = 5000;
        m_uiFrostBreathTimer        = 24000;
        m_uiWildMagicTimer          = 18000;
        m_uiSpectralBlastTimer      = 30000;

        m_uiExitTimer               = 0;

        m_bIsUncorrupted        = false;
        m_bIsBanished           = false;
        m_bIsEnraged            = false;
    }

    void JustReachedHome()
    {
		// Eject all players and set event to fail
		DoCastSpellIfCan(m_creature, SPELL_SPECTRAL_REALM_EJECT, CAST_TRIGGERED);
		if (m_pInstance)
			m_pInstance->SetData(TYPE_KALECGOS, FAIL);
    }

	void EnterEvadeMode()
    {
        // Check if the boss is uncorrupted when evading
        if (m_bIsUncorrupted)
            return;

        ScriptedAI::EnterEvadeMode();
    }

    void Aggro(Unit* pWho)
    {
        DoScriptText(SAY_EVIL_AGGRO, m_creature);

        if (m_pInstance)
            m_pInstance->SetData(TYPE_KALECGOS, IN_PROGRESS);
    }

    void DamageTaken(Unit* pDoneBy, uint32 &uiDamage)
    {
        if (uiDamage > m_creature->GetHealth())
        {
			uiDamage = 0;
            // If Sathrovarr is not banished yet, then banish the boss
            if (!m_bIsUncorrupted)
            {
                if (DoCastSpellIfCan(m_creature, SPELL_BANISH, CAST_TRIGGERED) == CAST_OK)
                {
                    m_bIsBanished = true;
                    m_creature->GetMotionMaster()->MoveIdle();
                }
            }
            else
				DoStartOutro();
        }
    }

    void KilledUnit(Unit* pVictim)
    {
        DoScriptText(urand(0, 1) ? SAY_EVIL_SLAY_1 : SAY_EVIL_SLAY_2, m_creature);
    }

    void DoStartOutro()
    {
        if (!m_pInstance)
            return;

		// Bring Sathrovarr in the normal realm
        if (Creature* pSathrovarr = m_pInstance->GetSingleCreatureFromStorage(NPC_SATHROVARR))
        {
            pSathrovarr->CastSpell(pSathrovarr, SPELL_TELEPORT_NORMAL_REALM, true);
            pSathrovarr->DealDamage(pSathrovarr, pSathrovarr->GetHealth(), NULL, DIRECT_DAMAGE, SPELL_SCHOOL_MASK_NORMAL, NULL, false);
        }

        if (Creature* pKalec = m_pInstance->GetSingleCreatureFromStorage(NPC_KALECGOS_HUMAN))
			pKalec->ForcedDespawn();

        EnterEvadeMode();
        m_creature->setFaction(35);
		m_creature->GetMotionMaster()->MoveIdle();
        DoScriptText(SAY_GOOD_PLRWIN, m_creature);
        m_uiExitTimer = 10000;
    }

    void MovementInform(uint32 uiMotionType, uint32 uiPointId)
    {
        if (uiMotionType != POINT_MOTION_TYPE)
            return;

        if (uiPointId)
        {
            if (m_pInstance)
                m_pInstance->SetData(TYPE_KALECGOS, DONE);

            m_creature->ForcedDespawn(1000);
        }
    }

    void UpdateAI(const uint32 uiDiff)
    {
        if (!m_creature->SelectHostileTarget() || !m_creature->getVictim())
            return;

        if (m_uiExitTimer)
        {
            if (m_uiExitTimer <= uiDiff)
            {
                float fX, fY, fZ;
                m_creature->GetRandomPoint(m_creature->GetPositionX(), m_creature->GetPositionY(), m_creature->GetPositionZ(), 30.0f, fX, fY, fZ);
                fZ = 70.0f;

            m_creature->SetByteValue(UNIT_FIELD_BYTES_1, 3, UNIT_BYTE1_FLAG_ALWAYS_STAND | UNIT_BYTE1_FLAG_UNK_2);
                m_creature->SetLevitate(true);
                m_creature->GetMotionMaster()->MovePoint(1, fX, fY, fZ);
                m_uiExitTimer = 0;
            }
            else
                m_uiExitTimer -= uiDiff;
        }

        if (m_bIsBanished)
        {
            // When Sathrovarr is banished then start outro
            if (m_bIsUncorrupted)
                DoStartOutro();

            // return when banished
            return;
        }

        if (!m_bIsEnraged && m_creature->GetHealthPercent() < 10.0f)
        {
            // If the boss already has the aura, then mark the enraged as true
            if (m_creature->HasAura(SPELL_CRAZED_RAGE))
                m_bIsEnraged = true;
            else
            {
                // Cast the crazed rage aura on Sathrovarr too
                if (Creature* pSathrovarr = m_pInstance->GetSingleCreatureFromStorage(NPC_SATHROVARR))
                    pSathrovarr->CastSpell(pSathrovarr, SPELL_CRAZED_RAGE, true);

                if (DoCastSpellIfCan(m_creature, SPELL_CRAZED_RAGE) == CAST_OK)
                    m_bIsEnraged = true;
            }
        }

        if (m_uiArcaneBuffetTimer < uiDiff)
        {
            if (DoCastSpellIfCan(m_creature->getVictim(), SPELL_ARCANE_BUFFET) == CAST_OK)
            {
                if (!urand(0, 2))
                    DoScriptText(SAY_EVIL_SPELL_1, m_creature);

                m_uiArcaneBuffetTimer = 20000;
            }
        }
        else
            m_uiArcaneBuffetTimer -= uiDiff;

        if (m_uiFrostBreathTimer < uiDiff)
        {
            if (DoCastSpellIfCan(m_creature->getVictim(), SPELL_FROST_BREATH) == CAST_OK)
            {
                if (!urand(0, 1))
                    DoScriptText(SAY_EVIL_SPELL_2, m_creature);

                m_uiFrostBreathTimer = 25000;
            }
        }
        else
            m_uiFrostBreathTimer -= uiDiff;

        if (m_uiTailLashTimer < uiDiff)
        {
            if (DoCastSpellIfCan(m_creature, SPELL_TAIL_LASH) == CAST_OK)
                m_uiTailLashTimer = urand(5000, 10000);
        }
        else
            m_uiTailLashTimer -= uiDiff;

        if (m_uiWildMagicTimer < uiDiff)
        {
            if (Unit* pTarget = m_creature->SelectAttackingTarget(ATTACKING_TARGET_RANDOM, 0))
            {
                if (DoCastSpellIfCan(pTarget, aWildMagicSpells[urand(0, 5)]) == CAST_OK)
                    m_uiWildMagicTimer = 19000;
            }
        }
        else
            m_uiWildMagicTimer -= uiDiff;

        if (m_uiSpectralBlastTimer < uiDiff)
        {
            if (DoCastSpellIfCan(m_creature, SPELL_SPECTRAL_BLAST) == CAST_OK)
                m_uiSpectralBlastTimer = 25000;
        }
        else
            m_uiSpectralBlastTimer -= uiDiff;

        DoMeleeAttackIfReady();
    }
};

struct MANGOS_DLL_DECL boss_sathrovarrAI : public ScriptedAI
{
    boss_sathrovarrAI(Creature* pCreature) : ScriptedAI(pCreature)
    {
        m_pInstance = (instance_sunwell_plateau*)pCreature->GetInstanceData();
        Reset();
    }

    instance_sunwell_plateau* m_pInstance;

    uint32 m_uiCorruptingStrikeTimer;
    uint32 m_uiCurseOfBoundlessAgonyTimer;
    uint32 m_uiShadowBoltVolleyTimer;
    bool m_bIsBanished;
    bool m_bIsEnraged;

    void Reset()
    {
        // FIXME: Timers
        m_uiCorruptingStrikeTimer       = 5000;
        m_uiCurseOfBoundlessAgonyTimer  = 15000;
        m_uiShadowBoltVolleyTimer       = 10000;

        m_bIsBanished = false;
        m_bIsEnraged  = false;

        DoCastSpellIfCan(m_creature, SPELL_SPECTRAL_INVISIBILITY);
    }

    void Aggro(Unit* pWho)
    {
        DoScriptText(SAY_SATH_AGGRO, m_creature);

        if (!m_pInstance)
            return;

        // Human Kalec starts to attack too
        if (Creature* pKalec = m_pInstance->GetSingleCreatureFromStorage(NPC_KALECGOS_HUMAN))
            pKalec->AI()->AttackStart(m_creature);
    }

    void DamageTaken(Unit* pDoneBy, uint32 &uiDamage)
    {
        if (uiDamage > m_creature->GetHealth())
        {
            uiDamage = 0;

            if (DoCastSpellIfCan(m_creature, SPELL_BANISH, CAST_TRIGGERED) == CAST_OK)
                m_bIsBanished = true;

            if (!m_pInstance)
                return;

            if (Creature* pKalecgos = m_pInstance->GetSingleCreatureFromStorage(NPC_KALECGOS_DRAGON))
            {
				// Eject all players and set Kalec to uncorrupted
                pKalecgos->CastSpell(pKalecgos, SPELL_SPECTRAL_REALM_EJECT, true);
                if (boss_kalecgosAI* pKalecgosAI = dynamic_cast<boss_kalecgosAI*>(pKalecgos->AI()))
                    pKalecgosAI->m_bIsUncorrupted = true;
            }
        }
    }

    void KilledUnit(Unit* pVictim)
    {
        DoScriptText(urand(0, 1) ? SAY_SATH_SLAY_1 : SAY_SATH_SLAY_2, m_creature);
    }

	void JustDied(Unit* pKiller)
    {
        DoScriptText(SAY_SATH_DEATH, m_creature);

		if (m_pInstance)
            m_pInstance->SetData(TYPE_KALECGOS, DONE);
    }

    void UpdateAI(const uint32 uiDiff)
    {
        if (!m_creature->SelectHostileTarget() || !m_creature->getVictim())
            return;

        if (m_bIsBanished)
            return;

        if (!m_bIsEnraged && m_creature->GetHealthPercent() < 10.0f)
        {
            // If the boss already has the aura, then mark the enraged as true
            if (m_creature->HasAura(SPELL_CRAZED_RAGE))
                m_bIsEnraged = true;
            else
            {
                if (Creature* pKalecgos = m_pInstance->GetSingleCreatureFromStorage(NPC_KALECGOS_DRAGON))
                    pKalecgos->CastSpell(pKalecgos, SPELL_CRAZED_RAGE, true);
				if (DoCastSpellIfCan(m_creature, SPELL_CRAZED_RAGE) == CAST_OK)
                    m_bIsEnraged = true;
            }
        }

        if (m_uiCorruptingStrikeTimer < uiDiff)
        {
            if (DoCastSpellIfCan(m_creature->getVictim(), SPELL_CORRUPTING_STRIKE) == CAST_OK)
            {
                if (!urand(0, 1))
                    DoScriptText(SAY_SATH_SPELL_2, m_creature);

                m_uiCorruptingStrikeTimer = 13000;
            }
        }
        else
            m_uiCorruptingStrikeTimer -= uiDiff;

        if (m_uiCurseOfBoundlessAgonyTimer < uiDiff)
        {
            if (Unit* pTarget = m_creature->SelectAttackingTarget(ATTACKING_TARGET_RANDOM, 0))
            {
                if (DoCastSpellIfCan(pTarget, SPELL_CURSE_OF_BOUNDLESS_AGONY) == CAST_OK)
                    m_uiCurseOfBoundlessAgonyTimer = 35000;
            }
        }
        else
            m_uiCurseOfBoundlessAgonyTimer -= uiDiff;

        if (m_uiShadowBoltVolleyTimer < uiDiff)
        {
            if (DoCastSpellIfCan(m_creature->getVictim(), SPELL_SHADOW_BOLT_VOLLEY) == CAST_OK)
            {
                if (!urand(0, 1))
                    DoScriptText(SAY_SATH_SPELL_1, m_creature);

                m_uiShadowBoltVolleyTimer = 15000;
            }
        }
        else
            m_uiShadowBoltVolleyTimer -= uiDiff;

        DoMeleeAttackIfReady();
    }
};

struct MANGOS_DLL_DECL boss_kalecgos_humanoidAI : public ScriptedAI
{
    boss_kalecgos_humanoidAI(Creature* pCreature) : ScriptedAI(pCreature)
    {
        m_pInstance = (instance_sunwell_plateau*)pCreature->GetInstanceData();
        Reset();
    }

    instance_sunwell_plateau* m_pInstance;

    uint32 m_uiRevitalizeTimer;
    uint32 m_uiHeroicStrikeTimer;

    bool m_bHasYelled10Percent;
    bool m_bHasYelled20Percent;

    void Reset()
    {
        //TODO: Times!
        m_uiRevitalizeTimer     = 30000;
        m_uiHeroicStrikeTimer   = 8000;

        m_bHasYelled10Percent   = false;
        m_bHasYelled20Percent   = false;

        DoCastSpellIfCan(m_creature, SPELL_SPECTRAL_INVISIBILITY);
    }

    void Aggro(Unit* pWho)
    {
        DoScriptText(SAY_GOOD_AGGRO, m_creature);
    }

    void JustDied(Unit* pKiller)
    {
        if (m_pInstance)
            m_pInstance->SetData(TYPE_KALECGOS, FAIL);
    }

    void UpdateAI(const uint32 uiDiff)
    {
        if (!m_creature->SelectHostileTarget() || !m_creature->getVictim())
            return;

        if (m_uiRevitalizeTimer < uiDiff)
        {
            // Cast on self because spell has target "all friendly units around the caster"
            if (DoCastSpellIfCan(m_creature, SPELL_REVITALIZE) == CAST_OK)
                m_uiRevitalizeTimer = 30000;
        }
        else
            m_uiRevitalizeTimer -= uiDiff;

        if (m_uiHeroicStrikeTimer < uiDiff)
        {
            if (DoCastSpellIfCan(m_creature->getVictim(), SPELL_HEROIC_STRIKE) == CAST_OK)
                m_uiHeroicStrikeTimer = 30000;
        }
        else
            m_uiHeroicStrikeTimer -= uiDiff;

        if (m_creature->GetHealthPercent() < 20.0f && !m_bHasYelled20Percent)
        {
            DoScriptText(SAY_GOOD_NEAR_DEATH_20, m_creature);
            m_bHasYelled20Percent = true;
        }

        if (m_creature->GetHealthPercent() < 10.0f && !m_bHasYelled10Percent)
        {
            DoScriptText(SAY_GOOD_NEAR_DEATH_10, m_creature);
            m_bHasYelled10Percent = true;
        }

        DoMeleeAttackIfReady();
    }
};

CreatureAI* GetAI_boss_kalecgos(Creature* pCreature)
{
    return new boss_kalecgosAI(pCreature);
}

CreatureAI* GetAI_boss_sathrovarr(Creature* pCreature)
{
    return new boss_sathrovarrAI(pCreature);
}

CreatureAI* GetAI_boss_kalecgos_humanoid(Creature* pCreature)
{
    return new boss_kalecgos_humanoidAI(pCreature);
}

void AddSC_boss_kalecgos()
{
    Script* pNewScript;

    pNewScript = new Script;
    pNewScript->Name = "boss_kalecgos";
    pNewScript->GetAI = &GetAI_boss_kalecgos;
    pNewScript->RegisterSelf();

    pNewScript = new Script;
    pNewScript->Name = "boss_sathrovarr";
    pNewScript->GetAI = &GetAI_boss_sathrovarr;
    pNewScript->RegisterSelf();

    pNewScript = new Script;
    pNewScript->Name = "boss_kalecgos_humanoid";
    pNewScript->GetAI = &GetAI_boss_kalecgos_humanoid;
    pNewScript->RegisterSelf();
}

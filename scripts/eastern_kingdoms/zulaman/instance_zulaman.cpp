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
SDName: Instance_Zulaman
SD%Complete: 50
SDComment: Support for Quests and Mini-Events still TODO
SDCategory: Zul'Aman
EndScriptData */

#include "precompiled.h"
#include "zulaman.h"

instance_zulaman::instance_zulaman(Map* pMap) : ScriptedInstance(pMap),
    m_uiEventTimer(MINUTE*IN_MILLISECONDS),
    m_uiGongCount(0),
	m_uiBearEventPhase(0),
    m_bIsBearPhaseInProgress(false),

    m_uiEggsRemainingCount_Left(20),
    m_uiEggsRemainingCount_Right(20)
{
    Initialize();
}

void instance_zulaman::Initialize()
{
    memset(&m_auiEncounter, 0, sizeof(m_auiEncounter));
    memset(&m_auiRandVendor, 0, sizeof(m_auiRandVendor));
}

bool instance_zulaman::IsEncounterInProgress() const
{
    // Skip Time-Event and Time-Event timer
    for (uint8 i = 1; i < MAX_ENCOUNTER - 1; ++i)
    {
        if (m_auiEncounter[i] == IN_PROGRESS)
            return true;
    }

    return false;
}

void instance_zulaman::OnCreatureCreate(Creature* pCreature)
{
    switch(pCreature->GetEntry())
    {
        case NPC_AKILZON:
        case NPC_HALAZZI:
		case NPC_NALORAKK:
        case NPC_MALACRASS:
        case NPC_HARRISON:
        case NPC_SPIRIT_LYNX:
        case NPC_BEAR_SPIRIT:
        case NPC_EAGLE_SPIRIT:
        case NPC_LYNX_SPIRIT:
        case NPC_DRAGONHAWK_SPIRIT:
			  // Insert Malacrass companions here for better handling
        case NPC_ALYSON:
        case NPC_THURG:
        case NPC_SLITHER:
        case NPC_RADAAN:
        case NPC_GAZAKROTH:
        case NPC_FENSTALKER:
        case NPC_DARKHEART:
        case NPC_KORAGG:
            m_mNpcEntryGuidStore[pCreature->GetEntry()] = pCreature->GetObjectGuid();
            break;

        case NPC_TANZAR:      m_aEventNpcInfo[INDEX_NALORAKK].npGuid = pCreature->GetObjectGuid(); break;
        case NPC_KRAZ:        m_aEventNpcInfo[INDEX_JANALAI].npGuid =  pCreature->GetObjectGuid(); break;
        case NPC_ASHLI:       m_aEventNpcInfo[INDEX_HALAZZI].npGuid =  pCreature->GetObjectGuid(); break;
        case NPC_HARKOR:      m_aEventNpcInfo[INDEX_AKILZON].npGuid =  pCreature->GetObjectGuid(); break;

        case NPC_EGG:
            if (m_auiEncounter[TYPE_JANALAI] != DONE)
                m_lEggsGUIDList.push_back(pCreature->GetObjectGuid());
            break;
			 case NPC_MEDICINE_MAN:
        case NPC_TRIBES_MAN:
        case NPC_WARBRINGER:
        case NPC_AXETHROWER:
            if (pCreature->GetPositionZ() > 10.0f && pCreature->GetPositionZ() < 15.0f)
                m_aNalorakkEvent[0].sBearTrashGuidSet.insert(pCreature->GetObjectGuid());
            else if (pCreature->GetPositionZ() > 25.0f && pCreature->GetPositionZ() < 30.0f)
                m_aNalorakkEvent[1].sBearTrashGuidSet.insert(pCreature->GetObjectGuid());
            else if (pCreature->GetPositionZ() > 40.0f && pCreature->GetPositionZ() < 41.0f)
                m_aNalorakkEvent[2].sBearTrashGuidSet.insert(pCreature->GetObjectGuid());
            else if (pCreature->GetPositionZ() > 41.0f)
                m_aNalorakkEvent[3].sBearTrashGuidSet.insert(pCreature->GetObjectGuid());
            break;
    }
}

void instance_zulaman::OnCreatureDeath(Creature* pCreature)	
{
    switch(pCreature->GetEntry())
    {	
        case NPC_MEDICINE_MAN:	
        case NPC_TRIBES_MAN:	
        case NPC_WARBRINGER:
        case NPC_AXETHROWER:	
            if (m_aNalorakkEvent[m_uiBearEventPhase].sBearTrashGuidSet.find(pCreature->GetObjectGuid()) != m_aNalorakkEvent[m_uiBearEventPhase].sBearTrashGuidSet.end())	
            {	
                ++m_aNalorakkEvent[m_uiBearEventPhase].uiTrashKilled;	
                if (m_aNalorakkEvent[m_uiBearEventPhase].uiTrashKilled == m_aNalorakkEvent[m_uiBearEventPhase].sBearTrashGuidSet.size())	
                {	
                    if (Creature* pNalorakk = GetSingleCreatureFromStorage(NPC_NALORAKK))	
                    {	
                        ++m_uiBearEventPhase;	
                        if (m_uiBearEventPhase == MAX_BEAR_WAVES)	
                            pNalorakk->RemoveFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_PASSIVE);	
                        else	
                        {	
                            pNalorakk->SetWalk(false);	
                            pNalorakk->GetMotionMaster()->MovePoint(1, aBearEventInfo[m_uiBearEventPhase].fX, aBearEventInfo[m_uiBearEventPhase].fY, aBearEventInfo[m_uiBearEventPhase].fZ);	
                        }	
                    }	
                }	
            }	
            break;	
    }	
}	

void instance_zulaman::OnCreatureEvade(Creature* pCreature)	
{	
    switch(pCreature->GetEntry())	
    {	
        case NPC_MEDICINE_MAN:	
        case NPC_TRIBES_MAN:	
        case NPC_WARBRINGER:	
        case NPC_AXETHROWER:	
            for (GUIDSet::const_iterator itr = m_aNalorakkEvent[m_uiBearEventPhase].sBearTrashGuidSet.begin(); itr != m_aNalorakkEvent[m_uiBearEventPhase].sBearTrashGuidSet.end(); ++itr)	
            {	
                Creature* pTemp = instance->GetCreature(*itr);	
                if (pTemp && !pTemp->isAlive())	
                    pTemp->Respawn();	
            }
            m_aNalorakkEvent[m_uiBearEventPhase].uiTrashKilled = 0;
            m_bIsBearPhaseInProgress = false;
			break;

     }

 }
void instance_zulaman::SendNextBearWave(Unit* pTarget)
{
    for (GUIDSet::const_iterator itr = m_aNalorakkEvent[m_uiBearEventPhase].sBearTrashGuidSet.begin(); itr != m_aNalorakkEvent[m_uiBearEventPhase].sBearTrashGuidSet.end(); ++itr)
    {
        Creature* pTemp = instance->GetCreature(*itr);
        if (pTemp && pTemp->isAlive())
        {
            pTemp->RemoveFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_PASSIVE);
            pTemp->AI()->AttackStart(pTarget);

            // For the first wave we need to make them jump to the ground before attacking
			//doesnt work in 2.4.3
           /* if (!m_uiBearEventPhase)
            {
                float fX, fY, fZ;
                pTemp->GetRandomPoint(35.31f, 1412.24f, 2.04f, 3.0f, fX, fY, fZ);
                pTemp->GetMotionMaster()->MoveJump(fX, fY, fZ, pTemp->GetSpeed(MOVE_RUN)*2, 5.0f);
            }
			*/
        }
    }

    m_bIsBearPhaseInProgress = true;
}
void instance_zulaman::OnObjectCreate(GameObject* pGo)
{
    switch(pGo->GetEntry())
    {
        case GO_STRANGE_GONG:
            break;
        case GO_MASSIVE_GATE:
            if (m_auiEncounter[TYPE_EVENT_RUN] == DONE || m_auiEncounter[TYPE_EVENT_RUN] == FAIL)
                pGo->SetGoState(GO_STATE_ACTIVE);
            break;
        case GO_WIND_DOOR:
            break;
        case GO_LYNX_TEMPLE_ENTRANCE:
			if (m_auiEncounter[TYPE_AKILZON] == DONE && m_auiEncounter[TYPE_NALORAKK] == DONE && m_auiEncounter[TYPE_JANALAI] == DONE)
                pGo->SetGoState(GO_STATE_ACTIVE);
            break;
        case GO_LYNX_TEMPLE_EXIT:
            if (m_auiEncounter[TYPE_HALAZZI] == DONE)
                pGo->SetGoState(GO_STATE_ACTIVE);
            break;
        case GO_HEXLORD_ENTRANCE:
            if (GetKilledPreBosses() == 4)
                pGo->SetGoState(GO_STATE_ACTIVE);
            break;
        case GO_WOODEN_DOOR:
            if (m_auiEncounter[TYPE_MALACRASS] == DONE)
                pGo->SetGoState(GO_STATE_ACTIVE);
            break;
        case GO_FIRE_DOOR:
            break;

		case GO_TANZARS_TRUNK:
        case GO_KRAZS_PACKAGE:
        case GO_ASHLIS_BAG:
        case GO_HARKORS_SATCHEL:

        case GO_TANZARS_CAGE:
        case GO_KRAZS_CAGE:
        case GO_ASHLIS_CAGE:
        case GO_HARKORS_CAGE:

        case GO_LOOT_BOX_DWARF:

        case GO_DWARF_HAMMER:
            break;

        default:
            return;
    }
    m_mGoEntryGuidStore[pGo->GetEntry()] = pGo->GetObjectGuid();
}

void instance_zulaman::SetData(uint32 uiType, uint32 uiData)
{
    debug_log("SD2: Instance Zulaman: SetData received for type %u with data %u",uiType,uiData);

    switch (uiType)
    {
        case TYPE_EVENT_RUN:
            if (uiData == SPECIAL)
            {
                ++m_uiGongCount;
                if (m_uiGongCount == 5)
                    m_auiEncounter[TYPE_EVENT_RUN] = uiData;
                return;
            }
            if (uiData == IN_PROGRESS)
            {
                DoTimeRunSay(RUN_START);
                DoUseDoorOrButton(GO_MASSIVE_GATE);
                if (m_auiEncounter[TYPE_RUN_EVENT_TIME])
                    SetData(TYPE_RUN_EVENT_TIME, m_auiEncounter[TYPE_RUN_EVENT_TIME]);
                else
                    SetData(TYPE_RUN_EVENT_TIME, 20);   // 20 Minutes as default time
                DoUpdateWorldState(WORLD_STATE_ID, 1);
            }
            if (uiData == FAIL)
            {
                DoTimeRunSay(RUN_FAIL);
                DoUpdateWorldState(WORLD_STATE_ID, 0);
				m_auiEncounter[TYPE_EVENT_RUN] = uiData;
                // Kill remaining Event NPCs
                for (uint8 i = 0; i < MAX_CHESTS; ++i)
                {
                    // Not yet rescued, so too late
                    if (!m_aEventNpcInfo[i].uiSavePosition)
                    {
                        if (Creature* pCreature = instance->GetCreature(m_aEventNpcInfo[i].npGuid))
						{
                            //Summon corpse of creature
                            switch(pCreature->GetEntry())
                            {
                                case NPC_TANZAR:
                                    pCreature->SummonCreature(NPC_TANZARS_CORPSE, 0.0f, 0.0f, 0.0f, 0.0f,TEMPSUMMON_CORPSE_DESPAWN, 0);
                                    break;
                                case NPC_KRAZ:
                                    pCreature->SummonCreature(NPC_KRAZS_CORPSE, 0.0f, 0.0f, 0.0f, 0.0f,TEMPSUMMON_CORPSE_DESPAWN, 0);
                                    break;
                                case NPC_ASHLI:
                                    pCreature->SummonCreature(NPC_ASHLIS_CORPSE, 0.0f, 0.0f, 0.0f, 0.0f,TEMPSUMMON_CORPSE_DESPAWN, 0);
                                    break;
                                case NPC_HARKOR:
                                    pCreature->SummonCreature(NPC_HARKORS_CORPSE, 0.0f, 0.0f, 0.0f, 0.0f,TEMPSUMMON_CORPSE_DESPAWN, 0);
                                    break;
                            }
                            pCreature->ForcedDespawn();
						}
                    }
                }
            }
            if (uiData == DONE)
            {
                DoTimeRunSay(RUN_DONE);
                DoUpdateWorldState(WORLD_STATE_ID, 0);
            }
            m_auiEncounter[uiType] = uiData;
            break;
        case TYPE_AKILZON:
            DoUseDoorOrButton(GO_WIND_DOOR);
            if (uiData == DONE)
            {
                if (m_auiEncounter[TYPE_EVENT_RUN] == IN_PROGRESS)
                {
                    m_auiEncounter[TYPE_RUN_EVENT_TIME] += 10; // Add 10 minutes
                    SetData(TYPE_RUN_EVENT_TIME, m_auiEncounter[TYPE_RUN_EVENT_TIME]);
                    DoChestEvent(INDEX_AKILZON);
                }
            }
            m_auiEncounter[uiType] = uiData;
            break;
        case TYPE_NALORAKK:
            if (uiData == DONE)
            {
                if (m_auiEncounter[TYPE_EVENT_RUN] == IN_PROGRESS)
                {
                    m_auiEncounter[TYPE_RUN_EVENT_TIME] += 15; // Add 15 minutes
                    SetData(TYPE_RUN_EVENT_TIME, m_auiEncounter[TYPE_RUN_EVENT_TIME]);
                    DoChestEvent(INDEX_NALORAKK);
                }
            }
            m_auiEncounter[uiType] = uiData;
            break;
        case TYPE_JANALAI:
            if (uiData == FAIL)
            {
                m_uiEggsRemainingCount_Left = 20;
                m_uiEggsRemainingCount_Right = 20;

                for(GUIDList::const_iterator itr = m_lEggsGUIDList.begin(); itr != m_lEggsGUIDList.end(); ++itr)
                {
                    if (Creature* pEgg = instance->GetCreature(*itr))
                    {
                        if (!pEgg->isAlive())
                            pEgg->Respawn();
                    }
                }
            }
            if (uiData == DONE)
            {
                m_lEggsGUIDList.clear();

                if (m_auiEncounter[TYPE_EVENT_RUN] == IN_PROGRESS)
                    DoChestEvent(INDEX_JANALAI);
            }
            m_auiEncounter[uiType] = uiData;
            break;
        case TYPE_HALAZZI:
            DoUseDoorOrButton(GO_LYNX_TEMPLE_ENTRANCE);
            if (uiData == DONE)
            {
                DoUseDoorOrButton(GO_LYNX_TEMPLE_EXIT);
                if (m_auiEncounter[TYPE_EVENT_RUN] == IN_PROGRESS)
                    DoChestEvent(INDEX_HALAZZI);
            }
            m_auiEncounter[uiType] = uiData;
            break;
        case TYPE_MALACRASS:
            DoUseDoorOrButton(GO_HEXLORD_ENTRANCE);
            if (uiData == DONE)
                DoUseDoorOrButton(GO_WOODEN_DOOR);
            m_auiEncounter[uiType] = uiData;
            break;
        case TYPE_ZULJIN:
            DoUseDoorOrButton(GO_FIRE_DOOR);
            m_auiEncounter[uiType] = uiData;
            break;
        case TYPE_RUN_EVENT_TIME:
            m_auiEncounter[uiType] = uiData;
            DoUpdateWorldState(WORLD_STATE_COUNTER, m_auiEncounter[uiType]);
            break;

        case TYPE_J_EGGS_RIGHT:
            --m_uiEggsRemainingCount_Right;
            break;
        case TYPE_J_EGGS_LEFT:
            --m_uiEggsRemainingCount_Left;
            break;
        case TYPE_RAND_VENDOR_1:
            m_auiRandVendor[0] = uiData;
            break;
        case TYPE_RAND_VENDOR_2:
            m_auiRandVendor[1] = uiData;
            break;

        default:
            error_log("SD2: Instance Zulaman: ERROR SetData = %u for type %u does not exist/not implemented.", uiType, uiData);
            return;
    }

	if (m_auiEncounter[TYPE_AKILZON] == DONE && m_auiEncounter[TYPE_NALORAKK] == DONE && m_auiEncounter[TYPE_JANALAI] == DONE)
    {
        DoUseDoorOrButton(GO_LYNX_TEMPLE_ENTRANCE);
    }

    if (uiData == DONE && GetKilledPreBosses() == 4 && (uiType == TYPE_AKILZON || uiType == TYPE_NALORAKK || uiType == TYPE_JANALAI || uiType == TYPE_HALAZZI))
    {
        DoUseDoorOrButton(GO_HEXLORD_ENTRANCE);
    }

    if (uiData == DONE || uiType == TYPE_RUN_EVENT_TIME || uiType == TYPE_EVENT_RUN)
    {
        OUT_SAVE_INST_DATA;

        std::ostringstream saveStream;
        saveStream << m_auiEncounter[TYPE_EVENT_RUN] << " " << m_auiEncounter[TYPE_AKILZON] << " " << m_auiEncounter[TYPE_NALORAKK] << " "
            << m_auiEncounter[TYPE_JANALAI] << " " << m_auiEncounter[TYPE_HALAZZI] << " " << m_auiEncounter[TYPE_MALACRASS] << " "
            << m_auiEncounter[TYPE_ZULJIN] << " " << m_auiEncounter[TYPE_RUN_EVENT_TIME];

        m_strInstData = saveStream.str();

        SaveToDB();
        OUT_SAVE_INST_DATA_COMPLETE;
    }
}

void instance_zulaman::Load(const char* chrIn)
{
    if (!chrIn)
    {
        OUT_LOAD_INST_DATA_FAIL;
        return;
    }

    OUT_LOAD_INST_DATA(chrIn);

    std::istringstream loadStream(chrIn);
    loadStream >> m_auiEncounter[TYPE_EVENT_RUN] >> m_auiEncounter[TYPE_AKILZON] >> m_auiEncounter[TYPE_NALORAKK] >> m_auiEncounter[TYPE_JANALAI]
        >> m_auiEncounter[TYPE_HALAZZI] >> m_auiEncounter[TYPE_ZULJIN] >> m_auiEncounter[TYPE_MALACRASS] >> m_auiEncounter[TYPE_RUN_EVENT_TIME];

    // Skip m_auiEncounter[TYPE_RUN_EVENT_TIME], to start the time event properly if needed
    for (uint8 i = 0; i < MAX_ENCOUNTER - 1; ++i)
    {
        if (m_auiEncounter[i] == IN_PROGRESS)
            m_auiEncounter[i] = NOT_STARTED;
    }

    // Restart TYPE_EVENT_RUN if was already started
    if (m_auiEncounter[TYPE_RUN_EVENT_TIME] != 0 && m_auiEncounter[TYPE_EVENT_RUN] != DONE && m_auiEncounter[TYPE_EVENT_RUN] != FAIL)
        SetData(TYPE_EVENT_RUN, IN_PROGRESS);

    OUT_LOAD_INST_DATA_COMPLETE;
}

uint32 instance_zulaman::GetData(uint32 uiType)
{
    switch (uiType)
    {
        case TYPE_EVENT_RUN:
        case TYPE_AKILZON:
        case TYPE_NALORAKK:
        case TYPE_JANALAI:
        case TYPE_HALAZZI:
        case TYPE_ZULJIN:
        case TYPE_MALACRASS:
        case TYPE_RUN_EVENT_TIME:
            return m_auiEncounter[uiType];
        case TYPE_RAND_VENDOR_1: return m_auiRandVendor[0];
        case TYPE_RAND_VENDOR_2: return m_auiRandVendor[1];
        case TYPE_J_EGGS_LEFT:   return m_uiEggsRemainingCount_Left;
        case TYPE_J_EGGS_RIGHT:  return m_uiEggsRemainingCount_Right;
        default:
            return 0;
    }
}

uint8 instance_zulaman::GetKilledPreBosses()
{
    return (m_auiEncounter[TYPE_AKILZON] == DONE ? 1 : 0) + (m_auiEncounter[TYPE_NALORAKK] == DONE ? 1 : 0) + (m_auiEncounter[TYPE_JANALAI] == DONE ? 1 : 0) + (m_auiEncounter[TYPE_HALAZZI] == DONE ? 1 : 0);
}

void instance_zulaman::DoTimeRunSay(RunEventSteps uiData)
{
    switch (uiData)
    {
        case RUN_START:     DoOrSimulateScriptTextForThisInstance(SAY_INST_BEGIN, NPC_MALACRASS); break;
        case RUN_FAIL:      DoOrSimulateScriptTextForThisInstance(urand(0, 1) ? SAY_INST_SACRIF1 : SAY_INST_SACRIF2, NPC_MALACRASS); break;
        case RUN_DONE:      DoOrSimulateScriptTextForThisInstance(SAY_INST_COMPLETE, NPC_MALACRASS); break;
        case RUN_PROGRESS:
            // This function is on progress called before the data is set to the array
            switch (GetKilledPreBosses() + 1)
            {
                case 1:     DoOrSimulateScriptTextForThisInstance(SAY_INST_PROGRESS_1, NPC_MALACRASS); break;
                case 2:     DoOrSimulateScriptTextForThisInstance(SAY_INST_PROGRESS_2, NPC_MALACRASS); break;
                case 3:     DoOrSimulateScriptTextForThisInstance(SAY_INST_PROGRESS_3, NPC_MALACRASS); break;
            }
            break;
        case RUN_FAIL_SOON:
            switch (GetKilledPreBosses())
            {
                case 0:     DoOrSimulateScriptTextForThisInstance(SAY_INST_WARN_1, NPC_MALACRASS); break;
                case 1:     DoOrSimulateScriptTextForThisInstance(SAY_INST_WARN_2, NPC_MALACRASS); break;
                case 2:     DoOrSimulateScriptTextForThisInstance(SAY_INST_WARN_3, NPC_MALACRASS); break;
                case 3:     DoOrSimulateScriptTextForThisInstance(SAY_INST_WARN_4, NPC_MALACRASS); break;
            }
            break;
    }
}

void instance_zulaman::DoChestEvent(BossToChestIndex uiIndex)
{
    // Store Order of this kill
    m_aEventNpcInfo[uiIndex].uiSavePosition = GetKilledPreBosses() + 1;

    // Do Yell
    DoTimeRunSay(RUN_PROGRESS);

    // related NPC:     m_aEventNpcInfo[uiIndex].npGuid
    // related Chest:   m_aEventNpcInfo[uiIndex]        // Not yet stored, because likely unneeded
}

void instance_zulaman::Update(uint32 uiDiff)
{
    if (m_auiEncounter[TYPE_EVENT_RUN] == IN_PROGRESS)
    {
        if (m_uiEventTimer <= uiDiff)
        {
            if (m_auiEncounter[TYPE_RUN_EVENT_TIME] == 5)   // TODO, verify 5min for warning texts
                DoTimeRunSay(RUN_FAIL_SOON);

			--m_auiEncounter[TYPE_RUN_EVENT_TIME];
            if (m_auiEncounter[TYPE_RUN_EVENT_TIME] == 0)
            {
                debug_log("SD2: Instance Zulaman: event time reach end, event failed.");
                SetData(TYPE_EVENT_RUN, FAIL);
                return;
            }

            SetData(TYPE_RUN_EVENT_TIME, m_auiEncounter[TYPE_RUN_EVENT_TIME]);
            debug_log("SD2: Instance Zulaman: minute decrease to %u.", m_auiEncounter[TYPE_RUN_EVENT_TIME]);

            m_uiEventTimer = MINUTE*IN_MILLISECONDS;
        }
        else
            m_uiEventTimer -= uiDiff;
    }
}

InstanceData* GetInstanceData_instance_zulaman(Map* pMap)
{
    return new instance_zulaman(pMap);
}

void AddSC_instance_zulaman()
{
    Script* pNewScript;

    pNewScript = new Script;
    pNewScript->Name = "instance_zulaman";
    pNewScript->GetInstanceData = &GetInstanceData_instance_zulaman;
    pNewScript->RegisterSelf();
}

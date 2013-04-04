/***************************************************************************** 
* 
* PROJECT: MTADiag
* LICENSE: GNU GPL v3
* FILE: Diag.h
* PURPOSE: Header for Diag namespace
* DEVELOPERS: Matthew "Towncivilian" Wolfe <ligushka@gmail.com>
* 
* 
* 
* Multi Theft Auto is available from http://www.multitheftauto.com/
* 
*****************************************************************************/ 

#pragma once

#include "Common.h"

#define CUR_MTA_VERSIONS 5 // beginning at 1; encompasses 1.1, 1.2, 1.3, 1.4, 1.5

// MTA install path
#define MTAPathValue	"Last Install Location"
#define MTA11PathSubKey "SOFTWARE\\Multi Theft Auto: San Andreas All\\1.1"
#define MTA12PathSubKey "SOFTWARE\\Multi Theft Auto: San Andreas All\\1.2"
#define MTA13PathSubKey "SOFTWARE\\Multi Theft Auto: San Andreas All\\1.3"
#define MTA14PathSubKey "SOFTWARE\\Multi Theft Auto: San Andreas All\\1.4"
#define MTA15PathSubKey "SOFTWARE\\Multi Theft Auto: San Andreas All\\1.5"

// GTA:SA path
#define MTAGTAPathValue  "GTA:SA Path"
#define MTAGTAPathSubKey "SOFTWARE\\Multi Theft Auto: San Andreas All\\Common"

// Long MTA version string
#define MTAVerValue    "mta-version-ext"
#define MTA11VerSubKey "SOFTWARE\\Multi Theft Auto: San Andreas All\\1.1\\Settings\\general"
#define MTA12VerSubKey "SOFTWARE\\Multi Theft Auto: San Andreas All\\1.2\\Settings\\general"
#define MTA13VerSubKey "SOFTWARE\\Multi Theft Auto: San Andreas All\\1.3\\Settings\\general"
#define MTA14VerSubKey "SOFTWARE\\Multi Theft Auto: San Andreas All\\1.4\\Settings\\general"
#define MTA15VerSubKey "SOFTWARE\\Multi Theft Auto: San Andreas All\\1.5\\Settings\\general"

// Nightly download URLs
#define MTA11NightlyURL "https://nightly.mtasa.com/?mtasa-1.1.1-rc-latest"
#define MTA12NightlyURL "https://nightly.mtasa.com/?mtasa-1.2-rc-latest"
#define MTA13NightlyURL "https://nightly.mtasa.com/?mtasa-1.3.1-rc-latest"
#define MTA14NightlyURL "https://nightly.mtasa.com/?mtasa-1.4-unstable-latest"
#define MTA15NightlyURL "https://nightly.mtasa.com/?mtasa-1.5-unstable-latest"

#define MTAVerURL "https://nightly.mtasa.com/ver/"

namespace Diag {

	void                    Begin                       ( void );
	void                    Cleanup                     ( bool deleteLog = false );

	// gather all currently installed MTA:SA versions and ask the user to pick between them if necessary
	void                    GeneratePaths               ( void );
	void                    GenerateGTAFiles            ( void );
	bool                    PollMTAVersions             ( void );
	void                    UserPickVersion             ( void );

	// gather MTA's path and version, and GTA:SA's path
	static std::string      GetMTAPath                  ( void );
	static std::string      GetGamePath                 ( void );
	static std::string      GetMTAVersion               ( void );

	// update MTA:SA to the latest nightly/unstable build
	// and update DirectX if necessary
	void                    UpdateDirectX               ( void );
	void                    UpdateMTA                   ( void );

	// information gathering functions
	void                    DoSystemCommandWithOutput   ( std::string command );
	void                    GetDir                      ( std::string directory );
	void                    QueryWMIC                   ( std::string, std::string = "", std::string = "", std::string = "" );

	// used for storing environment variables, current system time, files, and some paths
	static std::string      logFileName;
	static std::string      tempDir;
	static std::string      systemRoot;
	static std::string      programData;
	static SYSTEMTIME       sysTime;
	static BOOL             bIsWOW64;

	extern std::vector<std::string> files;

	static bool             bQuit;

	static std::string      MTAPath;
	static std::string      GTAPath;

	// store current MTA version when GetMTAVersion() is called, and store the original version to dump in the log file
	static std::string      MTAVersion;
	static std::string      MTAShortVersion;
	static std::string      OriginalMTAVersion;
	static bool             MTAUpdated;

	static std::string      MTAVersionsInstalled[CUR_MTA_VERSIONS]; // array to store paths of all MTA versions currently installed
	static int              MTAVerChoice;                           // stores user's choice of which MTA version to diagnose

	static std::string      PasteBinResult;                         // HTTP response

	static bool             CompatRemoved1;                         // were any compatibility mode registry settings removed?
	static bool             CompatRemoved2;                         // were any compatibility mode registry settings removed?
	static bool             DXUpdated;                              // was DirectX updated by MTADiag?

	static struct fileList
    {
		std::string szMd5;
		std::string szFilename;
    } fileList[] = {
         {"309d860fc8137e5fe9e7056c33b4b8be", "\\eax.dll"},
         //{"0bfc9ecd5398cbef80ff7afc51620a45", "\\gta_sa.exe"},
         {"0602f672ba595716e64ec4040e6de376", "\\ogg.dll"},
         {"05b6fdb1ff98a4ec75a58536a0c47b5e", "\\stream.ini"},
         {"2840f08dd9753a5b13c60d6d1c165c9a", "\\vorbis.dll"},
         {"2b7b803311d2b228f065c45d13e1aeb2", "\\vorbisFile.dll"},
         //{"809dcd55ee8b0c77292d65601a04aee6", "\\ReadMe\\Readme.txt"},
         {"4736b2c90b00981255f9507308ee9174", "\\anim\\ped.ifp"},
         {"4f15962340d51394d47a60e11cdbb608", "\\audio\\CONFIG\\AudioEventHistory.txt"},
         {"b45905c794677467644240aa9abc2f60", "\\audio\\CONFIG\\BankLkup.dat"},
         {"da40c568a349b58c78c2a8faf8da95a9", "\\audio\\CONFIG\\BankSlot.dat"},
         {"d676adc31b1d0a95631451344892ddd2", "\\audio\\CONFIG\\EventVol.dat"},
         {"db1e657a3baafbb86cd1b715c5282c66", "\\audio\\CONFIG\\PakFiles.dat"},
         {"6e65fd943ad6b0bbbc032e1f081ce699", "\\audio\\CONFIG\\StrmPaks.dat"},
         {"528e75d663b8bae072a01351081a2145", "\\audio\\CONFIG\\TrakLkup.dat"},
         {"1717fe0644d83f7464665808b4b71b80", "\\audio\\SFX\\FEET"},
         {"7813ccc099987ff9e51c136ed919f665", "\\audio\\SFX\\GENRL"},
         {"80629d549f026ef5d27b6ac9fa453f90", "\\audio\\SFX\\PAIN_A"},
         {"c32a4277b6872df2c8e630da8822778a", "\\audio\\SFX\\SCRIPT"},
         {"24cc5f759d63838852eefb65251b5989", "\\audio\\SFX\\SPC_EA"},
         {"3d0b77d5b48f450e118172d68317941c", "\\audio\\SFX\\SPC_FA"},
         {"e817e9ee417cd5d99de0dbf7dc04d869", "\\audio\\SFX\\SPC_GA"},
         {"b3c47f66781ce3db74c63c2676185765", "\\audio\\SFX\\SPC_NA"},
         {"86d9efc06f19308875148b9cf0f8e1c3", "\\audio\\SFX\\SPC_PA"},
         {"0f188011ba849665440dc03b62d6ca72", "\\audio\\streams\\AA"},
         {"3649bb4a3cb2439aeec00dfee685fc7f", "\\audio\\streams\\ADVERTS"},
         {"b7d437493764d9ba333e786ce10c1b67", "\\audio\\streams\\AMBIENCE"},
         {"e26d86c7805d090d8210086876d6c35c", "\\audio\\streams\\BEATS"},
         {"fe31259226e0b4a8a963c70840e1fe8f", "\\audio\\streams\\CH"},
         {"c40bd5d69c17d182b653db890cb38001", "\\audio\\streams\\CO"},
         {"900148b8141ea4c1e782c3a48dbfbf3b", "\\audio\\streams\\CR"},
         {"c25fcaa329b3d48f197ff4ed2a1d2a4d", "\\audio\\streams\\CUTSCENE"},
         {"9b4c18e4f3e82f0fee41e30b2ea2246a", "\\audio\\streams\\DS"},
         {"4e6f0c294db93ab7b086f14d0ee3cf54", "\\audio\\streams\\HC"},
         {"909e7c4a7a29473e3885a96f987d7221", "\\audio\\streams\\MH"},
         {"a1ec1cbe16dbb9f73022c6f33658abe2", "\\audio\\streams\\MR"},
         {"e48b03a281243aeeb021f4b5323965e5", "\\audio\\streams\\NJ"},
         {"49b83551c684e17164f2047dcba3e5aa", "\\audio\\streams\\RE"},
         {"7491dc5325854c7117af6e31900f38dd", "\\audio\\streams\\RG"},
         {"8b83278106608ac1954cc4fb688d0f52", "\\audio\\streams\\TK"},
         {"f638fae1023422aef37b22b336e7fdc6", "\\data\\AudioEvents.txt"},
         {"6a484b0b2356c524207d939487f1bff1", "\\data\\animgrp.dat"},
         {"f856ba3a4ba25ae10b561aa764fba0c4", "\\data\\animviewer.dat"},
         {"a98936b0f3523f23cad2eacc0eaf7a9b", "\\data\\ar_stats.dat"},
         {"2b33843e79bd113873a5d0fb02157749", "\\data\\carcols.dat"},
         {"63138ab62a10428a7c88f0be8ece094d", "\\data\\cargrp.dat"},
         {"6cbe845361e76aae35ddca300867cadf", "\\data\\carmods.dat"},
         {"8762637e580eb936111859ffa29bddb4", "\\data\\clothes.dat"},
         {"8e133355396761bd5cd16bf873154b30", "\\data\\default.dat"},
         {"5b6d75bae827e2d88f24f2be66a037bb", "\\data\\default.ide"},
         {"eb30c2a90d66d6f0bf5e3a7d5447ac01", "\\data\\fonts.dat"},
         {"3199fc8b81a4c5334a497508fe408afd", "\\data\\furnitur.dat"},
         {"795a9c013ee683e286768e06e4a5e2d7", "\\data\\gridref.dat"},
         {"2d2e4f7f05e2d82b25c88707096d3393", "\\data\\gta.dat"},
         {"012841ec691f84de4606ddcbff89e997", "\\data\\gta_quick.dat"},
         {"6868accef933f1855ec28ce193a78159", "\\data\\handling.cfg"},
         {"7df10bed5404a2f7669cdfaa47b8b81b", "\\data\\info.zon"},
         {"0b78b0b080b05d2de9228e0d23196aed", "\\data\\main.sc"},
         {"79d255c7a27bb49b50d680390e908e5a", "\\data\\map.zon"},
         {"b2f05657980e4a693f8ff5eadcbad8f8", "\\data\\melee.dat"},
         {"f152559cdaba5573e9f8aa78bf1d0fc2", "\\data\\numplate.dat"},
         {"46a5e7dff90078842e24d9de5e92cc3e", "\\data\\object.dat"},
         {"67d960dde13228d4818e0f144adafe4e", "\\data\\ped.dat"},
         {"fa1731066423ba0c584e757eda946f15", "\\data\\pedgrp.dat"},
         {"f7dea69fa6ab973479b9ef0cf05d3d98", "\\data\\peds.ide"},
         {"d722c90c92f3ad5c1b531596769f61cd", "\\data\\pedstats.dat"},
         {"a2713338dbbd55898a4195e4464c6b06", "\\data\\plants.dat"},
         {"48676fe82312f8f4a1bdf65c76719425", "\\data\\polydensity.dat"},
         {"a43f90361d1034c819a602171d8d66cb", "\\data\\popcycle.dat"},
         {"7229fa03d65f135bd569c3692d67c4b3", "\\data\\procobj.dat"},
         {"c1086eb6c0bfa36845f2026b68519f14", "\\data\\shopping.dat"},
         {"2ee5d9c1abb281f26f8cd00e9eefd65e", "\\data\\statdisp.dat"},
         {"9eb4e4e474abd5da2f3961a5ef549f9e", "\\data\\surface.dat"},
         {"c32c586e8ba35742e356e6525619f7c3", "\\data\\surfaud.dat"},
         {"605dd0beabccc797ce94a51a3e4a09eb", "\\data\\surfinfo.dat"},
         {"d66a121bc8f17a5b69e34b841744956c", "\\data\\timecyc.dat"},
         {"c91ce6b9f69578dc0fcd890f6147224c", "\\data\\timecycp.dat"},
         {"e3c231039048a30680b8f13fb51cc4ac", "\\data\\txdcut.ide"},
         {"bdc3a0fced2402c5bc61585714457d4b", "\\data\\vehicles.ide"},
         {"690400ecc92169d9eaddaaa948903efb", "\\data\\water.dat"},
         {"16fe5a3e8c57d02eb62a44a96d8b9d39", "\\data\\water1.dat"},
         {"0a9bb49003680364f9f9768e9bcea982", "\\data\\weapon.dat"},
         {"4383184825f1613669ca3355e315f1e9", "\\data\\Decision\\BLANK.ped"},
         {"e9eec8d526895a406b574a078de613ba", "\\data\\Decision\\Cop.ped"},
         {"b38e087d8b77152a984cf8a5164d6e97", "\\data\\Decision\\FLAT.ped"},
         {"38dfb77dc7343d470688014b4eabce27", "\\data\\Decision\\GROVE.ped"},
         {"2ed56525f52ee06e96fe05599bb6fab1", "\\data\\Decision\\GangMbr.ped"},
         {"2f413f8cf94aa074d4d2b35984c1b1fe", "\\data\\Decision\\Indoors.ped"},
         {"ecfce8c43aa3e27eaa063543b2c68891", "\\data\\Decision\\MISSION.grp"},
         {"4ebc62f3c473d949cecac27f98bb87aa", "\\data\\Decision\\MISSION.ped"},
         {"e4fa5caa1558f2945294a3652e6f0cde", "\\data\\Decision\\PedEvent.txt"},
         {"03ee2db935fe87152b9e3540f1ac509e", "\\data\\Decision\\m_empty.ped"},
         {"f3d578db43e1148e657211cb392b35cd", "\\data\\Decision\\m_infrm.ped"},
         {"cf979d9712f478d0deb92fbb11c6ff2e", "\\data\\Decision\\m_norm.ped"},
         {"8296a96da8498d8848a191d47ea75ab5", "\\data\\Decision\\m_std.ped"},
         {"cf979d9712f478d0deb92fbb11c6ff2e", "\\data\\Decision\\m_tough.ped"},
         {"cf979d9712f478d0deb92fbb11c6ff2e", "\\data\\Decision\\m_weak.ped"},
         {"2b2456482e8719e8c64877070fecee7f", "\\data\\Decision\\Allowed\\Cop.ped"},
         {"99ef637f82455921c9572fede370e33b", "\\data\\Decision\\Allowed\\Fireman.ped"},
         {"cd6e25e2a07fa5e1cc2f0d952c51f3af", "\\data\\Decision\\Allowed\\GangMbr.ped"},
         {"5071731f4fd49d79b0562c7dec9a673b", "\\data\\Decision\\Allowed\\Indoors.ped"},
         {"837dd1e06da29bf5d7210a9074164cf2", "\\data\\Decision\\Allowed\\MISSION.grp"},
         {"cda22c3bed5dd3742542084461082d24", "\\data\\Decision\\Allowed\\RANDOM.grp"},
         {"d6ed517e1e6809c6ad0e9e2c163f410e", "\\data\\Decision\\Allowed\\RANDOM.ped"},
         {"25cfafb3b7da432277bfa1291df4d58d", "\\data\\Decision\\Allowed\\RANDOM2.grp"},
         {"75d670db732344ec3f90e7db71b1e3a6", "\\data\\Decision\\Allowed\\R_Norm.ped"},
         {"b3e4ca143c1bbcbf99ebf70ef95e7343", "\\data\\Decision\\Allowed\\R_Tough.ped"},
         {"3cddd65754ad3c6ee8aec71b8a69b6c3", "\\data\\Decision\\Allowed\\R_Weak.ped"},
         {"149d0c778cab34b995d3958f44eeb18b", "\\data\\Decision\\Allowed\\m_empty.ped"},
         {"d6ed517e1e6809c6ad0e9e2c163f410e", "\\data\\Decision\\Allowed\\m_norm.ped"},
         {"50ec4c398f482bbe9428e1011b4bc0b2", "\\data\\Decision\\Allowed\\m_plyr.ped"},
         {"ea0e40c00071a4a9446c19b12bf5a035", "\\data\\Decision\\Allowed\\m_steal.ped"},
         {"be9ea5daacc227b8383b9c84a5b3fa9b", "\\data\\Decision\\Allowed\\m_tough.ped"},
         {"03dc3abafa80abec285adc0bddee6777", "\\data\\Decision\\Allowed\\m_weak.ped"},
         {"cb2fdafd51c78baed7d2a60470007401", "\\data\\Decision\\ChrisM\\CMblnk.ped"},
         {"cc4bce60ef1aac211340bd54ad08b2e1", "\\data\\Decision\\ChrisM\\m_std_cm.ped"},
         {"81c527d932e4949a3f0dce77caab1b5b", "\\data\\Decision\\Craig\\crack1.ped"},
         {"fbefb46d14ba4dd939c3781d8ebdc2b8", "\\data\\Decision\\Imran\\sci1_is.ped"},
         {"5ac444f731e87c911d5f6469c98a6684", "\\data\\Decision\\Imran\\std1_is.ped"},
         {"07b03be54f98eae5e60674f77f9e9b45", "\\data\\Decision\\Imran\\std2_is.ped"},
         {"de53187f1c9ba8b1efcb240e7c01a4e9", "\\data\\Decision\\andyd\\ADgrp.grp"},
         {"cfefbc0fdc988cafcd4a3bf6b13be064", "\\data\\Decision\\andyd\\ADtemp.ped"},
         {"4f7aa59ad04a276f28211fe3780bd4da", "\\data\\Decision\\chris\\maf5.ped"},
         {"da741c471b42859c99b3468bde1dc621", "\\data\\Decision\\chris\\ryder3.ped"},
         {"36e16f72d8be78bb8628478d5642860d", "\\data\\Decision\\david\\dam_sec.ped"},
         {"bf932fd285c05f708171b2e7cf0abe35", "\\data\\Decision\\david\\hei2_sc.ped"},
         {"4d5f2754d6236d7e8c765def78b0fd68", "\\data\\Icons\\app.ico"},
         {"ca533637e0a1aac05c9b14a98069c224", "\\data\\Icons\\bin.ico"},
         {"ffcc3a0d32517475bc83f08331169ada", "\\data\\Icons\\saicon.ICN"},
         {"ffcc3a0d32517475bc83f08331169ada", "\\data\\Icons\\saicon2.ICN"},
         {"ffcc3a0d32517475bc83f08331169ada", "\\data\\Icons\\saicon3.ICN"},
         {"b65284518b3bab328530eb0f2b969d86", "\\data\\Paths\\NODES0.DAT"},
         {"167712a4ec91fd811670b120ba13ec33", "\\data\\Paths\\NODES1.DAT"},
         {"da13b3a0422f2866cb3cab02a93296fe", "\\data\\Paths\\NODES10.DAT"},
         {"64fe7f8a080d130a864139231afe909b", "\\data\\Paths\\NODES11.DAT"},
         {"65b0308c682d35dc2248d06d7b2f82a0", "\\data\\Paths\\NODES12.DAT"},
         {"1c93aefecb76c9debaa1108768968e15", "\\data\\Paths\\NODES13.DAT"},
         {"2f10afdf126cbfc9783d0fd8c02c92da", "\\data\\Paths\\NODES14.DAT"},
         {"f909cd6bc5c0c02a8d08bb32d552e06d", "\\data\\Paths\\NODES15.DAT"},
         {"dd279df9b16789efd9344634a8184e3a", "\\data\\Paths\\NODES16.DAT"},
         {"60b973155b02b795edbca8cf2f48cf07", "\\data\\Paths\\NODES17.DAT"},
         {"24a912273c03a722d9b29059e1abf61e", "\\data\\Paths\\NODES18.DAT"},
         {"dd23c23e88ad4bd12756344956188bba", "\\data\\Paths\\NODES19.DAT"},
         {"64db455aa43312b28f34fbd09db4f55d", "\\data\\Paths\\NODES2.DAT"},
         {"cd58e71192e4d7ad1c67bfa101a86d16", "\\data\\Paths\\NODES20.DAT"},
         {"bde0c1f2af9f0aec26f2a67bef413a89", "\\data\\Paths\\NODES21.DAT"},
         {"d93eb526f700063dfd8ddf45a7c2231b", "\\data\\Paths\\NODES22.DAT"},
         {"2e1814398fd1371bb4670c69d68dfd02", "\\data\\Paths\\NODES23.DAT"},
         {"d4c1eebeeb2f1acf9c6ba8d56db3390a", "\\data\\Paths\\NODES24.DAT"},
         {"bfdcea53f8c7cd13e0103d30ded4fd40", "\\data\\Paths\\NODES25.DAT"},
         {"73fad39d504a6970bf0a0a41d8e142b3", "\\data\\Paths\\NODES26.DAT"},
         {"db9bd596ff327e2a7198c3c3765b9e8a", "\\data\\Paths\\NODES27.DAT"},
         {"7f9d7a2adee16653b0f9d55b52cb50f1", "\\data\\Paths\\NODES28.DAT"},
         {"d7b0c519cbd553924864d7791d85061e", "\\data\\Paths\\NODES29.DAT"},
         {"8c628e0cc44cf743d81a9ad27755a94a", "\\data\\Paths\\NODES3.DAT"},
         {"02725b9160cc15e1f02663541ecec7c6", "\\data\\Paths\\NODES30.DAT"},
         {"e5d4b0d863f2f5e8b10a2f16a457919f", "\\data\\Paths\\NODES31.DAT"},
         {"53497bbc84888a528124b05691bd7903", "\\data\\Paths\\NODES32.DAT"},
         {"ad1a010dca11b38a3f74ffc981486ac7", "\\data\\Paths\\NODES33.DAT"},
         {"e466fbdab93e2da7d770297e3142c158", "\\data\\Paths\\NODES34.DAT"},
         {"06103a5194cd3f80a627769717315147", "\\data\\Paths\\NODES35.DAT"},
         {"66a66969ff328780a55e07a09fe64c34", "\\data\\Paths\\NODES36.DAT"},
         {"b861531f7bb7055e4e6e93b4c6aa6b4b", "\\data\\Paths\\NODES37.DAT"},
         {"3295bbe0b6c3f2b64647355751765f1b", "\\data\\Paths\\NODES38.DAT"},
         {"fec2948d8e95c4a23be317525aab6957", "\\data\\Paths\\NODES39.DAT"},
         {"96efd245326aba02a8d01591c2e79fef", "\\data\\Paths\\NODES4.DAT"},
         {"54c2f1b1e04f5331155381f7c4ee3fd8", "\\data\\Paths\\NODES40.DAT"},
         {"f9b517cebe4494f8e835a3b687b06fb1", "\\data\\Paths\\NODES41.DAT"},
         {"082018bc5f7333f1fde17195aff579bc", "\\data\\Paths\\NODES42.DAT"},
         {"ee21132eccedda259d49daead3c53e8a", "\\data\\Paths\\NODES43.DAT"},
         {"78c1fb3169a5f28968bfdfd988f7025a", "\\data\\Paths\\NODES44.DAT"},
         {"ffd41ddb6e0f2d92a0fab07940dc3e1d", "\\data\\Paths\\NODES45.DAT"},
         {"20a67fef94f022e17ec92c3da34a0f51", "\\data\\Paths\\NODES46.DAT"},
         {"653ffffd5e3471963d96acaf385ae01c", "\\data\\Paths\\NODES47.DAT"},
         {"1d6a286a72226087de60ab004bf119f7", "\\data\\Paths\\NODES48.DAT"},
         {"575611902ad1d4915cd6f840f6c2e2e5", "\\data\\Paths\\NODES49.DAT"},
         {"26cdfc8dad2446f97089508553eb481a", "\\data\\Paths\\NODES5.DAT"},
         {"c48ed895638f1bfc741feaf92a6d03bb", "\\data\\Paths\\NODES50.DAT"},
         {"f0db1fb94f8c6e0471a3acdd9cf56d13", "\\data\\Paths\\NODES51.DAT"},
         {"e2ee621c10fbdaa7023f3cd96b214a15", "\\data\\Paths\\NODES52.DAT"},
         {"05bdf6c8a436620f42099cab1aeeac2c", "\\data\\Paths\\NODES53.DAT"},
         {"ba6795f4c40774a606a13750cdb8b197", "\\data\\Paths\\NODES54.DAT"},
         {"3f77ff5560909b8fc8aa81beea0e9d47", "\\data\\Paths\\NODES55.DAT"},
         {"86d5c93305910202e3492b6f53fcc211", "\\data\\Paths\\NODES56.DAT"},
         {"75a4f69d23923d217153f20f3c1e5818", "\\data\\Paths\\NODES57.DAT"},
         {"229d13a5928502a495a36a4726141b12", "\\data\\Paths\\NODES58.DAT"},
         {"ae72839f7689826ce0249f4332a70ea5", "\\data\\Paths\\NODES59.DAT"},
         {"1fba066612f8df915d1035fd7d1c4d43", "\\data\\Paths\\NODES6.DAT"},
         {"8eddde8839d3c78bd2b3c7c5bf245e53", "\\data\\Paths\\NODES60.DAT"},
         {"3bbb3336994f956c3b918a327ce75a02", "\\data\\Paths\\NODES61.DAT"},
         {"13c113c73d27eb9f6e3ba948155fc379", "\\data\\Paths\\NODES62.DAT"},
         {"d528b8e68486b7219bd781a9ab73a4ad", "\\data\\Paths\\NODES63.DAT"},
         {"d5ad2df76b43b7a0ff47bbca7d00a718", "\\data\\Paths\\NODES7.DAT"},
         {"aad80180f62367553cea7d74d8a2e268", "\\data\\Paths\\NODES8.DAT"},
         {"3426f2796fd22056238fdccde13817ca", "\\data\\Paths\\NODES9.DAT"},
         {"2dbd1fa4fc98e4cd0d84e647b3a12fa6", "\\data\\Paths\\ROADBLOX.DAT"},
         {"56d9d1bc00e836d4fe26cc4069f4ee3e", "\\data\\Paths\\spath0.dat"},
         {"16b7c65078f262893e18a5d626bbedbd", "\\data\\Paths\\tracks.dat"},
         {"f5c1f87d1ae814a12a2aeb51e4ef27e0", "\\data\\Paths\\tracks2.dat"},
         {"fc3e3e450c92a0cdcdf6d5bf26dda212", "\\data\\Paths\\tracks3.dat"},
         {"10066144ef14c40fff4d5bd3cc147360", "\\data\\Paths\\tracks4.dat"},
         {"e2ecc58a359a90ea0624c5210d2a8195", "\\data\\Paths\\train.dat"},
         {"a7f3dec85faa8048d3422bbe5270d265", "\\data\\Paths\\train2.dat"},
         {"bc3d7fc5a6927b61c10acda92e7e20c0", "\\data\\maps\\Audiozon.ipl"},
         {"7d723b80560f956bddb8d97ed66086b8", "\\data\\maps\\cull.ipl"},
         {"fbe5264b558576cff738291bc17a9c51", "\\data\\maps\\occlu.ipl"},
         {"a355e96c0102c3187fe75da90572b3f2", "\\data\\maps\\occluLA.ipl"},
         {"e89a5ae5ee074086862664b50f6881f5", "\\data\\maps\\occluint.ipl"},
         {"9ac2fb7ddddfe7f71a4faad8f71a7b98", "\\data\\maps\\occlusf.ipl"},
         {"394dc4170c928d4227f9c0e185d51261", "\\data\\maps\\occluveg.ipl"},
         {"37426c7d5218aa13aaec2f582aaaabc4", "\\data\\maps\\paths.ipl"},
         {"b38219724ec5eaa1dba1df4331389509", "\\data\\maps\\paths2.ipl"},
         {"1f5f7a824575552057fe7001c54c51a9", "\\data\\maps\\paths3.ipl"},
         {"73c948a8d373623524b4fca8a2b9c25a", "\\data\\maps\\paths4.ipl"},
         {"8f3d100baff8ee2088d0b74474175250", "\\data\\maps\\paths5.ipl"},
         {"1438622c076f6122ff6cdd03241b638c", "\\data\\maps\\tunnels.ipl"},
         {"46f2df900d7d79a68ac3eac499cb6f35", "\\data\\maps\\txd.ide"},
         {"6f046fe75467807c612c7f5f0d9bc90f", "\\data\\maps\\LA\\LAe.ide"},
         {"fa95160d9826c195e1b9d5128c20b35f", "\\data\\maps\\LA\\LAe.ipl"},
         {"99d656e3f7b484cf7231c01e42b02c8e", "\\data\\maps\\LA\\LAe2.ide"},
         {"ffa2c90e439f5bd7da317c1ad29a9495", "\\data\\maps\\LA\\LAe2.ipl"},
         {"7b9289601c4961f461dd1f2d8b2cc0fa", "\\data\\maps\\LA\\LAhills.ide"},
         {"4c30d0186a0c65d5be31f5a45965003a", "\\data\\maps\\LA\\LAhills.ipl"},
         {"b24ce30c982c7bc5b8fafc58b97c0dd7", "\\data\\maps\\LA\\LAn.ide"},
         {"3f3a9e01ca47388ed62e7bd10527572f", "\\data\\maps\\LA\\LAn.ipl"},
         {"a0e347f662168d4397e2bc1140ba4b2e", "\\data\\maps\\LA\\LAn2.ide"},
         {"dbcd933c5857dab676715b930f3eef1f", "\\data\\maps\\LA\\LAn2.ipl"},
         {"f73f76648dc35e2b7eed800bcf699757", "\\data\\maps\\LA\\LAs.ide"},
         {"4540df6c3a170ab9b4ff3acce5a9674f", "\\data\\maps\\LA\\LAs.ipl"},
         {"d6f69e02992be26e5606fe1b3d6c8be5", "\\data\\maps\\LA\\LAs2.ide"},
         {"cf63399c3f61e8a93e0e8dc7a09e766b", "\\data\\maps\\LA\\LAs2.ipl"},
         {"e3c998eb61ff77a86aeee8ac804d1a7e", "\\data\\maps\\LA\\LAw.ide"},
         {"1bfe3e1fabd610dc76ab0b44e26dacc6", "\\data\\maps\\LA\\LAw.ipl"},
         {"91ce9890027ca87158a5328417689004", "\\data\\maps\\LA\\LAw2.ide"},
         {"f220195b871833163f5c04856b03ae8d", "\\data\\maps\\LA\\LAw2.ipl"},
         {"ae01aa97caf5741d240a301ee2770915", "\\data\\maps\\LA\\LAxref.ide"},
         {"235545b8eb93dcffcb70c1dc2ff6d5fe", "\\data\\maps\\LA\\LaWn.ide"},
         {"0720120f5c025757ece1344df0d85a30", "\\data\\maps\\LA\\LaWn.ipl"},
         {"a245de4449fe41d4ed7440b80c193d06", "\\data\\maps\\SF\\SFSe.ide"},
         {"b275b70372a6d91643e3f585325f8ddc", "\\data\\maps\\SF\\SFSe.ipl"},
         {"a45f71b74a0dc25d83892dc4ba5fac3c", "\\data\\maps\\SF\\SFe.ide"},
         {"da55b24ac305b2779f46cc1509f444d8", "\\data\\maps\\SF\\SFe.ipl"},
         {"27566b5a6dce82513cfdcf65c82ca958", "\\data\\maps\\SF\\SFn.ide"},
         {"a2133c436d46396b4e1511a33bcb8870", "\\data\\maps\\SF\\SFn.ipl"},
         {"8b6341f47887183fbe475c14722d8e9f", "\\data\\maps\\SF\\SFs.ide"},
         {"518077ea0974332c8352eb9af931d197", "\\data\\maps\\SF\\SFs.ipl"},
         {"4103fe4fffa18c4845027fb3d7929296", "\\data\\maps\\SF\\SFw.ide"},
         {"e435f3d83ef4abcc4ce56c41767601ee", "\\data\\maps\\SF\\SFw.ipl"},
         {"23b00cc73b9564f739730c61a232353d", "\\data\\maps\\SF\\SFxref.ide"},
         {"77fc33d796a96c6d3b680e2f74e6739a", "\\data\\maps\\country\\countn2.ide"},
         {"0727df5077b9a6bb7e3e23c1c3990da9", "\\data\\maps\\country\\countn2.ipl"},
         {"ad384494c4d2d94683d2c51cef390395", "\\data\\maps\\country\\countryN.ide"},
         {"443490bda181e7f87e74fcc7704f2500", "\\data\\maps\\country\\countryN.ipl"},
         {"85792d5a12621e879cc59ed87db82480", "\\data\\maps\\country\\countryS.ide"},
         {"8d1bd4b5d337139ff3a953d44fc2b0e3", "\\data\\maps\\country\\countryS.ipl"},
         {"b8d0fdd9f7223ded76f4b5f6700fcb6f", "\\data\\maps\\country\\countryW.ide"},
         {"d0883386721ef7dfd9322069f2bddd9a", "\\data\\maps\\country\\countrye.ide"},
         {"5033cf4354baaad2a521031a1b318df0", "\\data\\maps\\country\\countrye.ipl"},
         {"a46e0560ee61446cbb4d531d99dec553", "\\data\\maps\\country\\countryw.ipl"},
         {"661831485cc61d2b99dca07431cf08e4", "\\data\\maps\\country\\counxref.ide"},
         {"d22010cd9522b19bf07efbc421872add", "\\data\\maps\\generic\\barriers.ide"},
         {"319e6aea03de0c05e075a1f15ed1ce8c", "\\data\\maps\\generic\\dynamic.ide"},
         {"46f6e2bcfaed43f10885961408691c4e", "\\data\\maps\\generic\\dynamic2.ide"},
         {"63d672310c0cf0efaa8b96c584dd407a", "\\data\\maps\\generic\\multiobj.ide"},
         {"bf592e31a663405116a68b63e7d2c49f", "\\data\\maps\\generic\\procobj.ide"},
         {"b9c84559de97b49ce2036498b3d504d5", "\\data\\maps\\generic\\vegepart.ide"},
         {"8cdc36bf580a82bf281c9d3a257d4742", "\\data\\maps\\interior\\gen_int1.ide"},
         {"eb13aff288ed3876354a326a22f29d93", "\\data\\maps\\interior\\gen_int1.ipl"},
         {"9b55e0b126ef22f3703b5ccf5bb1b174", "\\data\\maps\\interior\\gen_int2.ide"},
         {"5efb82b23a9462cbf4c1d0ba6dbc9fd4", "\\data\\maps\\interior\\gen_int2.ipl"},
         {"5119e846419e50bbc35bd57415d4376e", "\\data\\maps\\interior\\gen_int3.ide"},
         {"e14d644ed3a26d5711704f875f2c11e1", "\\data\\maps\\interior\\gen_int3.ipl"},
         {"16c7dba5af8ae61172599f29ed9dd6c0", "\\data\\maps\\interior\\gen_int4.ide"},
         {"339028d9c3aac53a8a28212179fced01", "\\data\\maps\\interior\\gen_int4.ipl"},
         {"1f183baa44e0b759c2917c34ac23d3b5", "\\data\\maps\\interior\\gen_int5.ide"},
         {"0acd219bca22d7cb8c8b50d738c3275c", "\\data\\maps\\interior\\gen_int5.ipl"},
         {"48a554b28c8045c21b6ed0905a76768f", "\\data\\maps\\interior\\gen_intb.ide"},
         {"663b75b0898db03687d5e6edb1d3b7f8", "\\data\\maps\\interior\\gen_intb.ipl"},
         {"b46f52a4e205996c24791b3e9ad012de", "\\data\\maps\\interior\\int_LA.ide"},
         {"2ae3c352e5de6e290dc631da053c5cfc", "\\data\\maps\\interior\\int_LA.ipl"},
         {"cdfdb64d5254a0fc689604d000d0e29c", "\\data\\maps\\interior\\int_SF.ide"},
         {"3e8cf0138f81f0c505f245e7bbff8b28", "\\data\\maps\\interior\\int_SF.ipl"},
         {"f811cdbcfd62ad7c8bf4be61c2d89855", "\\data\\maps\\interior\\int_cont.ide"},
         {"5331b6ee9cbf7a976f98dc2cce6992e3", "\\data\\maps\\interior\\int_cont.ipl"},
         {"3ffe6a366fcadba1a2fb3fb8166ceb31", "\\data\\maps\\interior\\int_veg.ide"},
         {"8d8030efa3a493324016a395fd180926", "\\data\\maps\\interior\\int_veg.ipl"},
         {"be0f534711073a19378cd30231b9d094", "\\data\\maps\\interior\\propext.ide"},
         {"b7ca66885d4fc34fe2f2083f8ed5d725", "\\data\\maps\\interior\\props.ide"},
         {"043a304b604db2e22d7464bcc36a41ce", "\\data\\maps\\interior\\props2.ide"},
         {"19af45eb13708b6b3ed9434a42e6a929", "\\data\\maps\\interior\\savehous.ide"},
         {"2e79a68217244d7d99f3790b63fe3267", "\\data\\maps\\interior\\savehous.ipl"},
         {"10953f74890c554dd368ff20bdbeac3d", "\\data\\maps\\interior\\stadint.ide"},
         {"242ece3c9c070faf6751e66bfb17531c", "\\data\\maps\\interior\\stadint.ipl"},
         {"521496c7b8e148bf65e15a2eb9cffdba", "\\data\\maps\\leveldes\\leveldes.ide"},
         {"4276ccba517ef022c45b39608699a8af", "\\data\\maps\\leveldes\\leveldes.ipl"},
         {"8ea3bf7c907763418a32fd7b42249d96", "\\data\\maps\\leveldes\\levelmap.ide"},
         {"4fb15ec9a9bd76e47842e01cdefa2585", "\\data\\maps\\leveldes\\levelmap.ipl"},
         {"4dda3ffcee83f83ca554d85817e52198", "\\data\\maps\\leveldes\\levelxre.ide"},
         {"0779f538229decee62071e53f41d4b93", "\\data\\maps\\leveldes\\seabed.ide"},
         {"fbedb38d8860a71d63d30e4c0f458c86", "\\data\\maps\\leveldes\\seabed.ipl"},
         {"6a35f16ac1b76f151be42c3860c81ffd", "\\data\\maps\\vegas\\VegasN.ide"},
         {"ed0cca34a8fdc556a7ca835ee0923b58", "\\data\\maps\\vegas\\VegasS.ide"},
         {"593313a65eb9f8905c61bd02296e4468", "\\data\\maps\\vegas\\VegasW.ide"},
         {"eea176558132ff026e2e5dac68ff9e5a", "\\data\\maps\\vegas\\vegasE.ide"},
         {"ea279d9a9d6e3addb981b5186da91424", "\\data\\maps\\vegas\\vegasE.ipl"},
         {"e6b6cc52ad19a5e93a45d659019f7b41", "\\data\\maps\\vegas\\vegasN.ipl"},
         {"ce4cb524d5bee74bab77029e5541b1ae", "\\data\\maps\\vegas\\vegasS.ipl"},
         {"43989ba645119a9b287fb0e3782245e7", "\\data\\maps\\vegas\\vegasW.ipl"},
         {"61c9cd72a43ca6f34788c4bde736431e", "\\data\\maps\\vegas\\vegaxref.ide"},
         {"789520fd41a60f0067c802c6f00d021b", "\\data\\maps\\vegas\\vegaxref.ipl"},
         {"e5f05eea1d6fb145bfa0d5f9950ddd54", "\\data\\maps\\veh_mods\\veh_mods.ide"},
         {"6143a72e8ff2974db14f65df65d952b0", "\\models\\effects.fxp"},
         {"0802650dfea37ed516e1c0f12ccb77d7", "\\models\\effectsPC.txd"},
         {"3ea286fb7d7086d353b42a8e2b021cea", "\\models\\fonts.txd"},
         {"7414ee5a8fa7a906f1c49b8897805e07", "\\models\\fronten1.txd"},
         {"386dae2e9f205ed2c75c4499503466f7", "\\models\\fronten2.txd"},
         {"af42eee4d2d71a83039eaee3f602be9a", "\\models\\fronten3.txd"},
         {"aa7ba893d292c6bf2aa5e16e0e6c8c1b", "\\models\\fronten_pc.txd"},
         {"18d2abd58e28c06b721197a0458d4405", "\\models\\hud.txd"},
         {"5ba1aa955cf55240b6dd6e0a25d28b57", "\\models\\misc.txd"},
         {"585f47abb0a6ea6c17d5a7638a1a07d9", "\\models\\particle.txd"},
         {"9ff145d936961fd37915c6ae186f6775", "\\models\\pcbtns.txd"},
         {"74288cbdd843c3cfb77b036a5614ae9d", "\\models\\coll\\peds.col"},
         {"c84c1a1b67d5fad3df75dd8d45fc576b", "\\models\\coll\\vehicles.col"},
         {"510e74e32b323eee54dd7a243b073808", "\\models\\coll\\weapons.col"},
         {"c43d23e5b11f4c3b152a250898e664a3", "\\models\\generic\\air_vlo.DFF"},
         {"8e0b690f080ea4ad45b11e56e2bd51af", "\\models\\generic\\arrow.DFF"},
         {"e3026d63c0904f60be5a777159954146", "\\models\\generic\\hoop.dff"},
         {"cf9bfea2ea8e9045fe554763bd41ab85", "\\models\\generic\\vehicle.txd"},
         {"ca1b56627abf153dbb1153900b482ea0", "\\models\\generic\\wheels.DFF"},
         {"c55fc1a89a9cfdc63e3dd02ae0b82607", "\\models\\generic\\wheels.txd"},
         {"c22311afde99c0f7984211ccc0f958b0", "\\models\\generic\\zonecylb.DFF"},
         {"be8763269599e91dcc596f13056d58dc", "\\models\\grass\\grass0_1.dff"},
         {"fe3b316979b03509278268b7479614f1", "\\models\\grass\\grass0_2.dff"},
         {"51d72ecadea1da6b5c4e1272b77d79fb", "\\models\\grass\\grass0_3.dff"},
         {"07a37a4e069aafb2eeeab56125ee21ed", "\\models\\grass\\grass0_4.dff"},
         {"84e3cdac0050a7ea9a87395728b99ac3", "\\models\\grass\\grass1_1.dff"},
         {"84e3cdac0050a7ea9a87395728b99ac3", "\\models\\grass\\grass1_2.dff"},
         {"84e3cdac0050a7ea9a87395728b99ac3", "\\models\\grass\\grass1_3.dff"},
         {"84e3cdac0050a7ea9a87395728b99ac3", "\\models\\grass\\grass1_4.dff"},
         {"84e3cdac0050a7ea9a87395728b99ac3", "\\models\\grass\\grass2_1.dff"},
         {"84e3cdac0050a7ea9a87395728b99ac3", "\\models\\grass\\grass2_2.dff"},
         {"84e3cdac0050a7ea9a87395728b99ac3", "\\models\\grass\\grass2_3.dff"},
         {"84e3cdac0050a7ea9a87395728b99ac3", "\\models\\grass\\grass2_4.dff"},
         {"84e3cdac0050a7ea9a87395728b99ac3", "\\models\\grass\\grass3_1.dff"},
         {"84e3cdac0050a7ea9a87395728b99ac3", "\\models\\grass\\grass3_2.dff"},
         {"84e3cdac0050a7ea9a87395728b99ac3", "\\models\\grass\\grass3_3.dff"},
         {"84e3cdac0050a7ea9a87395728b99ac3", "\\models\\grass\\grass3_4.dff"},
         {"e88432f1e188a4cfc6959ae645a4329f", "\\models\\grass\\plant1.dff"},
         {"15552e439a8daf86a6da252ba575381f", "\\models\\grass\\plant1.txd"},
         {"0af76785f962354f27a7707a3a96af08", "\\models\\txd\\INTRO3.TXD"},
         {"a24adf48b3470d78e8e8a2931494429c", "\\models\\txd\\LD_BEAT.txd"},
         {"6d2e90394334626fe6d505753dc01b29", "\\models\\txd\\LD_BUM.txd"},
         {"d71de9903f5deddb4c2a659a327a24bb", "\\models\\txd\\LD_CARD.txd"},
         {"8da7a3ed7368509c6635349a714b441a", "\\models\\txd\\LD_CHAT.txd"},
         {"e6fd533942139b5c8b27f9ac3943a463", "\\models\\txd\\LD_DRV.txd"},
         {"7d792a8ead9c39dee548032a1aba3a16", "\\models\\txd\\LD_DUAL.txd"},
         {"f1aeb67f9608a5ca4ba03356f9f86036", "\\models\\txd\\LD_NONE.txd"},
         {"eeffae83faf0a380150914d14f71a8f4", "\\models\\txd\\LD_OTB.txd"},
         {"ed15b78cb33af5f52bd3254fa56ed524", "\\models\\txd\\LD_OTB2.txd"},
         {"30c79c8f7bc058e2b6f418da631a01a4", "\\models\\txd\\LD_PLAN.txd"},
         {"746b91206836e4682cf1c45474a574ca", "\\models\\txd\\LD_POKE.txd"},
         {"7f0e00597aeeaf5f28a9c428f17dc8db", "\\models\\txd\\LD_POOL.txd"},
         {"442018c9effb38a9c31756fe7a02a7ba", "\\models\\txd\\LD_RACE.txd"},
         {"6974d8c0bcc3401c4383923d7a1a1b31", "\\models\\txd\\LD_RCE1.txd"},
         {"01a9050c733b718f321c035c9150ce60", "\\models\\txd\\LD_RCE2.txd"},
         {"b7501b94264618eff233fe08e1dc48c8", "\\models\\txd\\LD_RCE3.txd"},
         {"08515462022d78d73bdfbe28fd222ada", "\\models\\txd\\LD_RCE4.txd"},
         {"385fcc26b2f99930ecc1b8f74a423f7e", "\\models\\txd\\LD_RCE5.txd"},
         {"ed57a860ee4c23f0cab4f26fa2e4a161", "\\models\\txd\\LD_ROUL.txd"},
         {"1de99059110d71a8536b6b65e1460819", "\\models\\txd\\LD_SLOT.txd"},
         {"e0860f912a2c08dbdcc58259d9a10f29", "\\models\\txd\\LD_SPAC.txd"},
         {"57bf8b7afa0ed6e8d67d006128f8f621", "\\models\\txd\\LD_TATT.txd"},
         {"22016ed7dbc404e6ede9de32d604434b", "\\models\\txd\\LOADSCS.txd"},
         {"ae2d6da4010e858d435221d8eb622cda", "\\models\\txd\\LOADSUK.txd"},
         {"979cd51b6fab476324e683d954d95384", "\\models\\txd\\intro1.txd"},
         {"781e430c5b84349664fba51975b0fc71", "\\models\\txd\\intro2.txd"},
         {"eeacc47cef37e8793fd7be8b94eb195f", "\\models\\txd\\intro4.txd"},
         {"b520bcc4776ea47f5c28a5472d47f46e", "\\models\\txd\\ld_grav.txd"},
         {"6d1606d92cda98bc165671ac81dd7f3b", "\\models\\txd\\ld_shtr.txd"},
         {"4a19104217c2a03b6174bd1967fd32dd", "\\models\\txd\\load0uk.txd"},
         {"a3fd64bfcab845c937e22c6470a0e37a", "\\models\\txd\\loadsc0.txd"},
         {"6cdd429acbc9517eced2c750b11d5741", "\\models\\txd\\loadsc1.txd"},
         {"c1e36ceb3e1c17d95dfd89c5929b337e", "\\models\\txd\\loadsc10.txd"},
         {"68a2faca6263692e4fac6c3ca43b93ba", "\\models\\txd\\loadsc11.txd"},
         {"f33defb5e41d3624ed64a07007fae354", "\\models\\txd\\loadsc12.txd"},
         {"03391f19da3e1b258877c1c501138bd2", "\\models\\txd\\loadsc13.txd"},
         {"9ee5c939749fdd302936017f4f44a424", "\\models\\txd\\loadsc14.txd"},
         {"46376c00d3b65154ab636fa6146779b8", "\\models\\txd\\loadsc2.txd"},
         {"4f000737e6f21636060340956e3dc342", "\\models\\txd\\loadsc3.txd"},
         {"42980fee6470fae884692bf22655b95f", "\\models\\txd\\loadsc4.txd"},
         {"821838b60953b40030569d63485087ca", "\\models\\txd\\loadsc5.txd"},
         {"c12a3d6cb3749f7f604e952d30580530", "\\models\\txd\\loadsc6.txd"},
         {"c0e771d9623a4bd9181d76efad7c63a3", "\\models\\txd\\loadsc7.txd"},
         {"f52b98ff04e0b75b334c74b10faca0ef", "\\models\\txd\\loadsc8.txd"},
         {"b18538e772479ba326a8119087c35544", "\\models\\txd\\loadsc9.txd"},
         {"bf6531f0f555d96c33a744d90bee2da0", "\\models\\txd\\outro.txd"},
         {"3fa129dedbe400ea3fcbcf5e422e8717", "\\models\\txd\\splash1.txd"},
         {"3e341a50f3680525f86a170e50b0734c", "\\models\\txd\\splash2.txd"},
         {"112bfb3f6776df59a09f5392ca12b0f9", "\\models\\txd\\splash3.txd"},
         //{"497d6d1447c62e57c55db49444876fac", "\\movies\\GTAtitles.mpg"},
         //{"fe0ddfa024d1296597890f27d24ae335", "\\movies\\Logo.mpg"},
         {"6791e6e0ffa6317af8a0dff648c9633d", "\\text\\american.gxt"},
         {"3d069d3322809ccae92f315f53b5ad52", "\\text\\french.gxt"},
         {"7801fc85df72041ba497bade4b258da5", "\\text\\german.gxt"},
         {"fe4b7386f7149826ec682c5816f91ae4", "\\text\\italian.gxt"},
         {"71e0281b4ff0ec6f192a2bda4d00effd", "\\text\\spanish.gxt"},
    };
}

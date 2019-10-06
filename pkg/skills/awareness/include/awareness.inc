include "include/say";
include ":attributes:attributes";
include ":itemUtils:itemdesc";
include ":gumps:yesno";
include ":traps:traps";
include ":charactercreation:habilidades";


function examinarAlvo(who)
	SendSysMessageEx(who, "Selecione um alvo.", SSM_REQUEST);
	var targ := Target(who);
	if( !targ )
		SendSysMessageEx(who, "Cancelado.", SSM_FAIL);
		return 0;
	elseif ( !targ.isA(POLCLASS_MOBILE) && !targ.isA(POLCLASS_CONTAINER) && !targ.isA(POLCLASS_DOOR) )
		SendSysMessageEx(who, "Alvo invalido.", SSM_FAIL);
		return 0;
	endif

	var found := 0;
	if ( GetObjProperty(targ, "Disfarcado") )
		if ( SkillCheck(who, AWARENESS, -1) > 0 )
			found:=1;
			var char := GetObjProperty(targ, "chardata");
			var name := char.FirstName;
			var last_name := char.LastName;
			if (last_name)
				name := name+" "+last_name;
			endif
			SendSysMessageEx(who, "Você descobriu que "+targ.name+" é, na verdade, "+name+".", SSM_INFO);
			SendSysMessageEx(who, "ATENCAO: Se voce nao conhece esse personagem, ignore o fato de saber o nome dele.", SSM_INFO);
		endif
	elseif ( GetObjProperty(targ, "TrapList") )
		if ( GetObjProperty(targ, "trapSpoted") )
			SendSysMessageEx(who, "Armadilha ja foi encontrada e esta obvia.", SSM_INFO);
			found:=1;
		else
		var diff;
		var lvl := Cint(GetObjProperty(targ, "level"));
		case (lvl)
			1: diff := 15;
			2: diff := 40;
			3: diff := 60;
			4: diff := 86;
			5: diff := 90;
			6: diff := 110;
			default: diff := 40; 
		endcase
			if ( SkillCheck(who, AWARENESS, diff) > 0 )
				found :=1;
				var revelar := YesNo(who, "Você descobriu uma armadilha, deseja revelá-la?", "Sim.", "Não, deixarei a armadilha escondida.");
				if (revelar)
					PrintText(targ, "*armadilha revelada por "+who.name+"*");
					TrapFound(targ, who);
				endif
			endif
		endif
	endif

	return found;
endfunction

function CalculateDHDiff(who, mobile)

	var mobile_skill := CInt(AP_GetSkill(mobile, SNEAK));

	//sendsysmessage(mobile, "dificuldade de  "  + who.name + "te achar " + mobile_skill);
	if (GetObjProperty(mobile, "#overridehiding") != error)
		mobile_skill := cint(GetObjProperty(mobile, "#overridehiding"));
	endif
	var dist := Distance(who, mobile);
	var difficulty;

	if (TemHabilidade(mobile, "Comunhao com as Sombras"))
		if ( dist >= 5 )
			difficulty := mobile_skill * 1.2;
		else
			difficulty := mobile_skill;
		endif
	else
		if (dist >=8)
			difficulty := mobile_skill;
		elseif (dist >= 7)
			difficulty := mobile_skill*0.95;
		elseif (dist >= 6)
			difficulty := mobile_skill*0.9;
		elseif ( dist >= 5 )
			difficulty := mobile_skill*0.85;
		elseif ( dist >= 4 )
			difficulty := CInt(mobile_skill * 0.8);
		elseif ( dist >= 3 )
				difficulty := CInt(mobile_skill * 0.7);
		elseif ( dist >= 2 )
			difficulty := CInt(mobile_skill * 0.6);
		elseif ( dist >= 1 )
			difficulty := CInt(mobile_skill * 0.5);
		else
			difficulty := CInt(mobile_skill * 0.25);
		endif
	endif

	return CInt(difficulty);

endfunction

function examinarArea(who)
	var range := CInt( AP_GetSkill(who, AWARENESS) / 10 ) + 1;

	var vantagem := (GetObjProperty(who, "chardata")).vantagemracial;
	if ( vantagem == "Sentidos Aguçados" )
		range := range + 5;
	endif

/*	var raca := GetObjProperty(who, "raca");
	if ( raca == "elfo" )
		range := range + 10;
	elseif ( raca == "meio-elfo" || raca == "drow" )
		range := range + 5;
	endif
*/
	var found := 0;
	foreach mobile in ListMobilesNearLocationEX(who.x, who.y, who.z, range, LISTEX_FLAG_HIDDEN)
		if ( mobile == who ); // Do Nothing
		elseif ( !mobile.npctemplate && CInt(GetObjProperty(who, "IgnoreHidden")) ); // Do Nothing
		else
			if ( CheckLineOfSight(who, mobile) )
			
				var difficulty := CalculateDHDiff(who, mobile);

				if ( SkillCheck(who, AWARENESS, difficulty) > 0 )
					found := found+1;
					mobile.hidden := 0;
					PrintText(mobile, "*aparece*");
					SendSysMessageEx(who, "Voce encontrou "+mobile.name+".", SSM_INFO);
					SendSysMessageEx(mobile, who.name+" te encontrou!", SSM_FAIL);
				endif
			endif
		endif
		sleepms(2);
	endforeach

	foreach item in ListItemsNearLocation(who.x, who.y, who.z, range)
		if ( item.invisible )
			if ( item.isTrap() )
				if ( SkillCheck(who, AWARENESS, -1) > 0 )
					found := found+1;
					item.invisible := 0;
					SendSysMessageEx(who, "Voce encontrou "+item.desc+".", SSM_INFO);
					PrintTextAbovePrivate(item, "*!*", who);
				endif
			else
				var diff := CInt(GetObjProperty(item, "FindDiff"));
				if ( diff )
					if ( SkillCheck(who, AWARENESS, diff) > 0 )
						found := found+1;
						item.invisible := 0;
						SendSysMessageEx(who, "Voce encontrou "+item.desc+".", SSM_INFO);
						PrintTextAbovePrivate(item, "*!*", who);
					endif
				endif
			endif
		endif
		sleepms(2);
	endforeach

	return found;
endfunction

#!/usr/bin/env /bin/bash 

_jack_completions()
{
	local cur prev pprev 

	cur=${COMP_WORDS[COMP_CWORD]}
	prev=${COMP_WORDS[COMP_CWORD-1]}

	if [ $COMP_CWORD -eq 1 ] ; then 

		COMPREPLY=($(compgen -W "aer ld mctp port set show" -- $cur))

	elif [ $COMP_CWORD -eq 2 ] ; then 

		case $prev in 
			aer) 	;;
			ld) 	COMPREPLY=($(compgen -W "config mem" -- $cur)) ;;
		 	mctp) 	;;
		 	port) 	COMPREPLY=($(compgen -W "bind config connect control disconnect unbind" -- $cur)) ;;
		 	set) 	COMPREPLY=($(compgen -W "ld limit qos" -- $cur)) ;;
		 	show) 	COMPREPLY=($(compgen -W "bos identity ld limit port qos switch vcs" -- $cur)) ;;
			*)		;;
		esac

	elif [ $COMP_CWORD -eq 3 ] ; then 

		pprev=${COMP_WORDS[COMP_CWORD-2]}

		if [ $pprev = "set" ] ; then 
			case $prev in 
				ld) 	COMPREPLY=($(compgen -W "allocations" -- $cur)) ;;	
				qos) 	COMPREPLY=($(compgen -W "allocated control limit" -- $cur)) ;;	
				*)		;;
			esac
		elif [ $pprev = "show" ] ; then 
			case $prev in 
				ld) 	COMPREPLY=($(compgen -W "allocations info" -- $cur)) ;;	
				qos) 	COMPREPLY=($(compgen -W "allocated control limit status" -- $cur)) ;;	
				*)		;;
			esac
		fi
	fi
}
complete -F _jack_completions jack


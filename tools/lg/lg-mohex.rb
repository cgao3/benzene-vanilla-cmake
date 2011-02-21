#!/usr/bin/ruby
require 'tools/lg/lg-hex'
class MoHexBot < BenzeneBot
    LOGIN='MoHexBot'
    BOSS_ID='MoHexBot'
    def initialize
        @supported_gametypes = /Hex 13x13/
        print "Enter Password for MoHexBot: "
        ps=gets
        ps=ps.chomp
        super(LOGIN,ps,BOSS_ID)
    end
    def genmove(size, moves)
        gtp = GTPClient.new(@logger, "src/mohex/mohex")
        sleep 1
        gtp.cmd('boardsize ' + size.to_s)
        gtp.cmd('param_game on_little_golem 1')
        gtp.cmd('param_game allow_swap 1')
        gtp.cmd('param_mohex knowledge_threshold 0')
        gtp.cmd('param_mohex reuse_subtree 0')
        gtp.cmd('param_mohex max_games 999999999')
        gtp.cmd('param_mohex max_time 120')
        gtp.cmd('param_mohex max_memory 28000000000')
        gtp.cmd('param_mohex num_threads 7')
        translate_LG2Hex!(moves)
        gtp.cmd('play-game ' + moves.join(' '))
        gtp.cmd('showboard')
        response = gtp.cmd('genmove ' + (moves.length % 2 == 0 ? 'b' : 'w'))
        gtp.cmd('quit')
        sleep 1
        gtp.close
        return response[2..-3]
    end
end

#enable to cause the http library to show warnings/errors
#$VERBOSE = 1
w=MoHexBot.new
loop {
    begin
        while w.parse
        end
        sleep(30)
    rescue Timeout::Error => e
        p 'timeout error (rbuff_fill exception), try again in 30 seconds'
		sleep(30)
    rescue => e
        p e.message
        p e.backtrace
        p 'An error... wait 5 minutes'
        sleep(300)
    end
}


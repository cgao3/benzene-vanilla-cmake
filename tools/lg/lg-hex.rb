#!/usr/bin/ruby

require 'lg-interface'

class GTPClient
    def initialize(cmdline)
        @io=IO.popen(cmdline,'w+')
    end
    def cmd(c)
        @io.puts c.strip
        return @io.gets("\n\n")
    end
    def close
        @io.close_write
        @io.close_read
    end
end

class UALGHex < LittleGolemInterface
    LOGIN='WolveBot'
    BOSS_ID='WolveBot'
    #gacohota
    def initialize
        @supported_gametypes = /Hex/
        print "Enter Password for WolveBot: "
        ps=gets
        ps=ps.chomp
        super(LOGIN,ps,BOSS_ID)
    end
    def coord_Hex2LG(c)
        return c if c == "swap"
        x=c[0].chr
        y=c[1..2].to_i
        y+=64
        c=(x+y.chr).downcase
        if (@swap)
            c.reverse!
        end
        return c
    end
    def call_hex(size, moves)
        gtp = GTPClient.new("../../src/wolve/wolve")
        gtp.cmd('boardsize ' + size.to_s)
        translate_LG2Hex!(moves)
        color='B'
        moves.each do |m|
            gtp.cmd("play #{color} #{m}")
            color = (color=='W'?'B':'W')
        end
        response = gtp.cmd('genmove '+color)
        gtp.cmd('quit')
        sleep 1
        return s
    end
    def parse_make_moves(gameids)
        gameids.each do |g|
            if (game = get_game(g))
                size = game.scan(/SZ\[(.+?)\]/).flatten[0].to_i
                moves = game.scan(/;[B|W]\[(.+?)\]/).flatten.map{|m| coord_HGF2GA(m, size) }
                self.log("Game #{g}, size #{size}: #{moves.join(' ')}")
                newmove = self.call_hex(size, moves)
                self.log("Game #{g}, size #{size}: response #{newmove}")
                self.post_move(g, coord_Hex2LG(newmove))
                @swap = false
            else
                self.log('error getting game')
                sleep(600)
            end
        end
    end
    def translate_LG2Hex!(moves)
        if (moves.include?("swap"))
            # we'll need to mirror our move back to LG later
            @swap = true
            moves.delete("swap")
            # mirror everything after the first move
            moves[1..-1].map! { |m| m.reverse! }
        end
        # f10 |-> FJ
        moves.map! do |m|
            m.upcase!
            x=m[0].chr
            y=m[1]-64
            m=x+y.to_s
        end
    end
end

#enable to cause the http library to show warnings/errors
#$VERBOSE = 1

w=UALGHex.new
#w.test_coords
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


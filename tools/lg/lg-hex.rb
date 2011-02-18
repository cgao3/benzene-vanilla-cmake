#!/usr/bin/ruby

require 'lg-interface'

class UALGHex < LittleGolemInterface
    #DEFAULT_LEVEL=12
    def initialize (params)
        @supported_gametypes = /Hex/
        @path = params[:executable]
        login = params[:user]
        pass = params[:pass]
        boss = params[:boss]
        sleep = params[:sleep] || 30
        super(login,pass,boss,sleep)
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
    def call_hex(size,moves)
        io=IO.popen("#{@path}",'w+')
        io.puts('boardsize '+size.to_s)
        io.puts('')
        io.puts('clear_board')
        io.puts('time_left white 10000')
        io.puts('time_left black 10000')
        # black moves first.
        color='B'
        # convert LG coords to our coords, and mirror moves if necessary
        translate_LG2Hex!(moves)
        # play out the game so far
        moves.each do |m|
            io.puts("play #{color} #{m}")
            color = (color=='W'?'B':'W')
        end
        io.puts('genmove '+color)
        io.puts('quit')
        io.close_write
        s=io.readlines
        io.close_read
        s=s.detect{ |i| i =~ /=\s[a-z]\d+|=\sswap/ }
        s=s[/[a-z]\d+|swap/]
        sleep 1
        return s
    end
    def parse_make_moves(gameids)
        gameids.each do |g|
            #fill config data with the defaults
            if get_game(g)
                #white starts
                begin
                    move=(@last_data.slice(/;.\[..\]\)/)[1].chr=='W') ? 'white' : 'black'
                rescue
                    #game hasn't begun, we're white
                    move='black'
                end
                if move=='black' then
                    opponent_nm=@last_data.scan(/PW\[(.+?)\]/).flatten.to_s
                else
                    opponent_nm=@last_data.scan(/PB\[(.+?)\]/).flatten.to_s
                end
                size=@last_data.scan(/SZ\[(.+?)\]/).flatten.to_s
                moves=@last_data.scan(/;[B|W]\[(.+?)\]/)
                zet=self.call_hex(size,moves.flatten)
                self.log("Played #{zet} (#{coord_Hex2LG(zet)}) in game #{g}".blue)
                self.post_move(g,coord_Hex2LG(zet))
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

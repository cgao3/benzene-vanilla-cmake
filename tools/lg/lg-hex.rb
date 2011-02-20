#!/usr/bin/ruby
require 'tools/lg/lg-interface'
require 'open3'

#class GTPClient
#    def initialize(cmdline)
#        @io=IO.popen(cmdline,'w+')
#    end
#    def cmd(c)
#        @io.puts c.strip
#        return @io.gets("\n\n")
#    end
#    def close
#        @io.close_write
#        @io.close_read
#    end
#end

class GTPClient
    def initialize(logger, cmdline)
        @logger = logger
        @ins, @out, @err = Open3.popen3(cmdline)
        Thread.new do  
            until (line = @err.gets).nil? do  
                log line
            end  
        end
    end
    def log(str)
        @logger.log_nostamp str
    end

    def cmd(c)
        log '>>' + c.strip
        @ins.puts c.strip
        response = @out.gets("\n\n")
        log '<<' + response
        return response
    end
    def close
        @ins.close_write
        @out.close_read
        @err.close_read
    end
end

class BenzeneBot < LittleGolemInterface
    def translate_Hex2LG(c)
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
    def translate_LG2Hex!(moves)
        if (moves.include?("swap"))
            # we'll need to mirror our move back to LG later
            @swap = true
            moves.delete("swap")
            # mirror everything after the first move
            moves[1..-1].map! { |m| m.reverse! }
        end
        # convert numbers to letters, eg: f10 to FJ
        moves.map! do |m|
            m.upcase!
            x=m[0].chr
            y=m[1]-64
            m=x+y.to_s
        end
    end
    def parse_make_moves(gameids)
        gameids.each do |g|
            if (game = get_game(g))
                size = game.scan(/SZ\[(.+?)\]/).flatten[0].to_i
                moves = game.scan(/;[B|W]\[(.+?)\]/).flatten
                self.log("Game #{g}, size #{size}: #{moves.join(' ')}")
                newmove = self.genmove(size, moves)
                self.log("Game #{g}, size #{size}: response #{newmove}")
                self.post_move(g, translate_Hex2LG(newmove))
                @swap = false
            else
                self.log('error getting game')
                sleep(600)
            end
        end
    end
end

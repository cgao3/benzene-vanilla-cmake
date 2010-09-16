#!/usr/bin/ruby -W0

require 'littlegoleminterface'

#################################################
# THIS IS FOR HAVANNAH, CONVERT TO HEX!!!!
#################################################

class Wanderer < LittleGolemInterface
  #DEFAULT_LEVEL=12
  LOGIN='wanderer_c'
  PSW='????'
  BOSS_ID='????'
  def initialize
    @supported_gametypes = /Hav/
    super(LOGIN,PSW,BOSS_ID)
  end
  def coord_GA2LG(c,size)
    #oo=middelpunt van LG
    return c if c == 'swap'
    c.downcase!
    x=c[0]-96
    x = (x<size ? 'o'[0]-(size-x) : 'o'[0]+(x-size)).chr
    y=c[1..2].to_i
    y=(y<size ? 'o'[0]+(size-y) : 'o'[0]-(y-size)).chr
    x+y
  end
  def coord_GA2HGF(c,size)

#GA
#A      +  o  o  o  o  o  +
#B      o  .  .  .  .  .  .  o
#C      o  .  .  .  .  .  .  .  o
#D      o  .  .  .  .  .  .  .  .  o
#E      o  .  .  .  .  .  .  .  .  .  o
#F      o  .  .  .  .  .  .  .  .  .  .  o
#G      +  .  .  .  .  .  .  .  .  .  .  .  +
#H         o  .  .  .  .  .  .  .  .  .  .  o
#I            o  .  .  .  .  .  .  .  .  .  o
#J               o  .  .  .  .  .  .  .  .  o
#K                  o  .  .  .  .  .  .  .  o
#L                     o  .  .  .  .  .  .  o
#M                        +  o  o  o  o  o  +
#       1  2  3  4  5  6  7  8  9 10 11 12 13
#White to move

#HGF
#      A  B  C  D  E  F  G  H  I  j  K  L  M
#      +  o  o  o  o  o  +
#      o  .  .  .  .  .  .  o
#      o  .  .  .  .  .  .  .  o
#      o  .  .  .  .  .  .  .  .  o
#      o  .  .  .  .  .  .  .  .  .  o
#      o  .  .  .  .  .  .  .  .  .  .  o
#      +  .  .  .  .  .  .  .  .  .  .  .  +
#         o  .  .  .  .  .  .  .  .  .  .  o
#            o  .  .  .  .  .  .  .  .  .  o
#               o  .  .  .  .  .  .  .  .  o
#                  o  .  .  .  .  .  .  .  o
#                     o  .  .  .  .  .  .  o
#                        +  o  o  o  o  o  +
#                         \  \  \  \  \  \  \
#                          7  6  5  4  3  2  1
#White to move

    return c if c == 'swap'
    c.upcase!
    x=c[0].chr
    y=c[1..2].to_i
    y-=(c[0]-64)-size if size < (c[0]-64)
    x+y.to_s
  end
  def coord_HGF2GA(c,size)
    return c if c == 'swap'
    c.upcase!
    x=c[0].chr
    y=c[1..2].to_i
    y+=(c[0]-64)-size if size < (c[0]-64)
    x+y.to_s
  end
  def call_wanderer(size,moves)
    io=IO.popen("./wanlin --ttt=#{5000}",'w+')
    io.puts('boardsize '+size.to_s)
    io.puts('')
    io.puts('clear_board')
    io.puts('time_left white 10000 1')
    io.puts('time_left black 10000 1')
    color='B'
    moves.each do |m|
      io.puts("play #{color} #{coord_HGF2GA(m,size.to_i)}")
      color = (color=='W'?'B':'W')
    end
    io.puts('genmove '+color)
    io.puts('quit')
    sleep 1
    io.close_write
    s=io.readlines
    io.close_read
    s=s.detect{|i| i =~ /=\s[A-Z]\d+|=\sswap/ }
    s=s[/[A-Z]\d+|swap/]
    sleep 1
    return s
  end
  def get_move_list(gameid)
    if get_game(gameid)
      #wit begint
      begin
        aanzet=(@last_data.slice(/;.\[..\]\)/)[1].chr=='W') ? 'white' : 'black'
      rescue
        #bij begin van het spel
        aanzet='white'
      end
      if aanzet=='black' then
        opponent_nm=@last_data.scan(/PW\[(.+?)\]/).flatten.to_s
      else
        opponent_nm=@last_data.scan(/PB\[(.+?)\]/).flatten.to_s
      end
      size=@last_data.scan(/SZ\[(.+?)\]/).flatten.to_s
      moves=@last_data.scan(/;[B|W]\[(.+?)\]/)
      return {:aanzet=>aanzet,:opponent_nm=>opponent_nm,:size=>size,:moves=>moves}
    else
      self.log('error getting game')
    end
  end
  def get_wanderer_move(size,moves)
    zet=self.call_wanderer(size,moves.flatten)
    return coord_GA2LG(zet,size.to_i)
  end
  def parse_make_moves(gameids)
    gameids.each do |g|
      #fill config data with the defaults
      if get_game(g)
        #wit begint
        begin
          aanzet=(@last_data.slice(/;.\[..\]\)/)[1].chr=='W') ? 'white' : 'black'
        rescue
          #bij begin van het spel
          aanzet='white'
        end
        if aanzet=='black' then
          opponent_nm=@last_data.scan(/PW\[(.+?)\]/).flatten.to_s
        else
          opponent_nm=@last_data.scan(/PB\[(.+?)\]/).flatten.to_s
        end
        size=@last_data.scan(/SZ\[(.+?)\]/).flatten.to_s
        moves=@last_data.scan(/;[B|W]\[(.+?)\]/)
        zet=self.call_wanderer(size,moves.flatten)
        self.post_move(g,coord_GA2LG(zet,size.to_i))
      else
        self.log('error getting game')
        sleep(600)
      end
    end
  end
end




w=Wanderer.new
while true
  begin
    while w.parse
    end
    sleep(30)
  rescue Timeout::Error => e
    p 'timeout error (rbuff_fill exception), try again in 30 seconds'
    sleep(30)
  rescue
    p 'An error... wait 5 minutes'
    sleep(300)
  end
end


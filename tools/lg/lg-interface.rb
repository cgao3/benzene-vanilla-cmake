#!/usr/bin/ruby
require 'net/http'
require 'yaml'

#fix rbuf_fill error
#http://www.ruby-forum.com/topic/105212
#module Net
#  class BufferedIO
#    def rbuf_fill
#      timeout(@read_timeout,ProtocolError) {
#      @rbuf << @io.sysread(BUFSIZE)
#      }
#    end
#  end
#end

class String
    def red
        "\e[31m#{self}\e[0m"
    end
    def red_back
        "\e[41m#{self}\e[0m"
    end
    def green
        "\e[32m#{self}\e[0m"
    end  
    def blue
    "\e[34m#{self}\e[0m"
    end
    def yellow
        "\e[33m#{self}\e[0m"
    end
end

class Logger
    def log(msg)
        puts '['+Time::now.strftime('%d-%m-%y %H:%M:%S')+'] '+msg
    end
end

class LittleGolemInterface
    def initialize (loginname,psw,boss_id,sleep=30)
        @login,@psw,@boss_id,@sleep=loginname,psw,boss_id,sleep
        @http = Net::HTTP.new('www.littlegolem.net')
        @config_data = {}
        @logger=Logger.new
    end
    def get_game(gid)
        path="/servlet/sgf/#{gid}/game.hgf"
        headers = {'Cookie' => @cookie}
        resp, @last_data = @http.get(path, headers)
        return (resp.code=='200')
    end
    def get_invitations
        path='/jsp/invitation/index.jsp'
        headers = {'Cookie' => @cookie}
        resp, @last_data = @http.get(path, headers)
        return (resp.code=='200')
    end
    def send_message(pid,title,msg)
        path="/jsp/message/new.jsp"
        @headers = {'Cookie' => @cookie}
        resp, @last_data=@http.post(path,"messagetitle=#{title}&message=#{msg}&plto=#{pid}",@headers)
        return (resp.code=='200')
    end
    def post_move(gid,mv,chat='')
        chat.sub!('+',' leads with ')
        path="/jsp/game/game.jsp?sendgame=#{gid}&sendmove=#{mv}"
        @headers = {'Cookie' => @cookie}
        resp, @last_data=@http.post(path,"message=#{chat}",@headers)
        if resp.code!='200'
            login
            logout
            resp, @last_data=@http.post(path,"message=#{chat}",@headers)
        end
        return (resp.code=='200')
    end
    def reply_invitation(inv_id,answer)
        path="/Invitation.action?#{answer}=&invid=#{inv_id}"
        headers = {'Cookie' => @cookie}
        resp, @last_data=@http.get(path, headers)
        return (resp.code=='200')
    end
    def log(msg)
        @logger.log(msg)
    end
    def logout
        path="/jsp/login/logoff.jsp"
        headers = {'Cookie' => @cookie}
        resp, @last_data=@http.get(path, headers)
        return (resp.code=='200')
    end
    def login
        path='/jsp/login/index.jsp'
        resp, data = @http.get(path, nil)
        @cookie = resp.response['set-cookie']
        data = "login=#{@login}&password=#{@psw}"
        headers = {'Cookie' => @cookie}
        resp, @last_data = @http.post(path, data, headers)
        return (resp.code=='200')
        # Output on the screen -> we should get either a 302 redirect (after a successful login) or an error page
        #puts 'Code = ' + resp.code #200=OK
        #puts 'Message = ' + resp.message #OK
        #resp.each {|key, val| puts key + ' = ' + val}
    end
    def get_gamesheet
        path='/jsp/game/index.jsp'
        headers = {'Cookie' => @cookie}
        resp, @last_data = @http.get(path, headers)
        return (resp.code=='200')
    end
    def get_my_turn_games
        if self.login 
            if get_gamesheet
                gamesheet=@last_data
                if !(gamesheet =~  /Games where it is your turn \[0\]/)
                    return gamesheet.slice(/your turn.*your opponent/m).scan(/gid=(\d+)?/).flatten
                end  
            end 
        else
            self.log("Could not log in, #{@sleep}s sleep".red_back)
        end
        []
    end
    def parse
        made_move=false
        if self.login and get_gamesheet
            gamesheet=@last_data
            if !(gamesheet =~  /Games where it is your turn \[0\]/)
                gameids=gamesheet.slice(/your turn.*your opponent/m).scan(/gid=(\d+)?/).flatten
                self.parse_make_moves(gameids)
                made_move=true
            else
                self.log("No games found where it's my turn, #{@sleep}s sleep")
            end
        else
            self.log('Login failed, trying again in 10 minutes.'.red_back)
            sleep(600)
        end
        #check invitations
        if gamesheet =~ /New invitations:/
            self.log('new invitation!'.green)
            if self.get_invitations
                #a=@last_data.slice(/Your decision.*?Confirm selection/m).scan(/<td>(.*?)<\/td>/m).flatten
                a=@last_data.slice(/Your decision.*?table>/m).scan(/<td>(.*?)<\/td>/m).flatten
                gametype=a[2]
                if gametype =~ @supported_gametypes
                    answer='accept'
                else
                    answer='refuse'
                end
                if (@boss_id)
                    self.send_message(@boss_id,"New invitation","#{gametype} #{answer}")
                end
                s=a[5]
                inv_id=s.scan(/invid=(\d*)?/m)[0]
                self.reply_invitation(inv_id,answer)
            end
        end
        made_move
    end
end

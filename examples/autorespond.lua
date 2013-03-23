-- EXAMPLE LUA SCRIPT
-- This simple script responds to any instant message with the text of the message
-- recieved reversed.

-- SAMPLE ARGUMENT
-- msg = { text = "Sample instant message",
--         protocol_id = "jabber",
--         remote_from_group = "Buddies",
--         remote_username = "user1@gmail.com/5223DFE",
--         remote_alias = "User1",
--         remote_status_id = "away",
--         local_username = "user2@gmail.com/2223DFE", 
--         local_alias  = "User2",
--         local_status_id = "available" 
--       }

function autorespond(msg)
    return(string.reverse(msg.text))
end

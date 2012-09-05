import Control.Applicative
import qualified Data.Map as M
import Data.Ratio
import qualified Data.Text as T
import System.Console.GetOpt
import System.Environment
import System.Exit
import System.IO
import Text.JSON
import Text.StringTemplate

main :: IO ()
main = do
    args <- getArgs
    case getOpt RequireOrder options args of
        (actions, [dataFile], [])     -> processTemplate actions (readFile dataFile)
        (actions, [],         [])     -> processTemplate actions getContents
        (_,        _:_:_,     errors) -> usage $ "Too many arguments\n" : errors
        (_,        _,         errors) -> usage errors

usage :: [String] -> IO ()
usage errors = do
    progName <- getProgName
    mapM_ (hPutStr stderr) errors
    hPutStrLn stderr $ usageInfo ("\nUsage: " ++ progName ++ " -t TEMPLATE [OPTIONS...] [DATA_FILE]") options
    exitFailure

data Config = Config {
    groupPath :: Maybe FilePath,
    templateName :: String
  }

defaultConfig :: Config
defaultConfig = Config {
    groupPath = Nothing,
    templateName = ""
  }

options :: [OptDescr (Config -> Config)]
options = [
    Option ['g'] ["group"]
        (ReqArg (\arg opts -> opts {groupPath = Just arg}) "DIR")
        "Template group directory",

    Option ['t'] ["template"]
        (ReqArg (\arg opts -> opts {templateName = arg}) "TEMPLATE")
        "Template name if using groups, otherwise template path (stdin if not given)"
  ]

processTemplate :: [Config -> Config] -> IO String -> IO ()
processTemplate actions getData = do
    let config = foldl (flip id) defaultConfig actions
    modelJSON <- getData
    template <- getTemplate config
    let modelResult = decode $ strip modelJSON :: Result JSValue
    case modelResult of
        Ok model -> putStrLn . render $ withContext template model
        Error err -> hPutStrLn stderr $ "Error while parsing JSON: " ++ err

getTemplate :: Config -> IO (StringTemplate String)
getTemplate config = case templateName config of
    "" -> do
        usage ["Missing template option"]
        fail "Missing template option"
    name -> case groupPath config of
        Nothing -> newSTMP <$> readFile name
        Just templateGroup -> do
            group <- directoryGroup templateGroup
            case getStringTemplate name group of
                Just template -> return template
                Nothing -> hPutStrLn stderr ("Error trying to get template " ++ name) >> exitFailure

strip :: String -> String
strip = T.unpack . T.strip . T.pack

instance ToSElem JSValue where
    toSElem JSNull = toSElem (Nothing :: Maybe String)
    toSElem (JSString s) = toSElem (fromJSString s)
    toSElem (JSBool b) = toSElem b
    toSElem (JSRational _ r)
        | denominator r == 1 = toSElem (numerator r)
        | otherwise          = toSElem (fromRational r :: Double)
    toSElem (JSArray l) = toSElem l
    toSElem (JSObject o) = toSElem . M.fromList $ fromJSObject o

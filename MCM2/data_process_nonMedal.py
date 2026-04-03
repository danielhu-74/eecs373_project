import pandas as pd
import numpy as np
import chardet

def detect_encoding(file_path):
    with open(file_path, 'rb') as f:
        raw_data = f.read()
        result = chardet.detect(raw_data)
        return result['encoding']


def read_csv(file_path):
    encoding = detect_encoding(file_path)
    return pd.read_csv(file_path, encoding=encoding)
# read data
data_dict = read_csv('./2025_Problem_C_Data/data_dictionary.csv')
athletes = read_csv('./2025_Problem_C_Data/summerOly_athletes.csv')
hosts = read_csv('./2025_Problem_C_Data/summerOly_hosts.csv')
medals = read_csv('./2025_Problem_C_Data/summerOly_medal_counts.csv')
programs = read_csv('./2025_Problem_C_Data/summerOly_programs.csv')

# fill missing values with 0
athletes.fillna(0, inplace=True)

# ensure the data types are correct
medals['Gold'] = medals['Gold'].astype(int) # 确保奖牌数为整数

# delete columns that are not needed
athletes.drop_duplicates(inplace=True)

# map the country names to the correct names
country_mapping = {
    'Soviet Union': 'Russia',
    'West Germany': 'Germany',
    'East Germany': 'Germany',
    'Yugoslavia': 'Serbia',
    'Czechoslovakia': 'Czech Republic',
    'Bohemia': 'Czech Republic',
    'Russian Empire': 'Russia',
    'United Team of Germany': 'Germany',
    'Unified Team': 'Russia',
    'Serbia and Montenegro': 'Serbia',
    'Netherlands Antilles': 'Netherlands',
    'Virgin Islands': 'United States',
}

noc_mapping = {
    'URS': 'RUS',
    'EUA': 'GER',
    'FRG': 'GER',
    'GDR': 'GER',
    'YUG': 'SRB',
    'TCH': 'CZE',
    'BOH': 'CZE',
    'EUN': 'RUS',
    'SCG': 'SRB',
    'ANZ': 'AUS',
    'NBO': 'KEN',
    'WIF': 'USA',
    'IOP': 'IOA',
}

country_codes = {
    'AFG': 'Afghanistan',
    'ALB': 'Albania',
    'ALG': 'Algeria',
    'AND': 'Andorra',
    'ANG': 'Angola',
    'ANT': 'Antigua and Barbuda',
    'ARG': 'Argentina',
    'ARM': 'Armenia',
    'ARU': 'Aruba',
    'ASA': 'American Samoa',
    'AUS': 'Australia',
    'AUT': 'Austria',
    'AZE': 'Azerbaijan',
    'BAH': 'Bahamas',
    'BAN': 'Bangladesh',
    'BAR': 'Barbados',
    'BDI': 'Burundi',
    'BEL': 'Belgium',
    'BEN': 'Benin',
    'BER': 'Bermuda',
    'BHU': 'Bhutan',
    'BIH': 'Bosnia and Herzegovina',
    'BIZ': 'Belize',
    'BLR': 'Belarus',
    'BOL': 'Bolivia',
    'BOT': 'Botswana',
    'BRA': 'Brazil',
    'BRN': 'Bahrain',
    'BRU': 'Brunei',
    'BUL': 'Bulgaria',
    'BUR': 'Burkina Faso',
    'CAF': 'Central African Republic',
    'CAM': 'Cambodia',
    'CAN': 'Canada',
    'CAY': 'Cayman Islands',
    'CGO': 'Congo',
    'CHA': 'Chad',
    'CHI': 'Chile',
    'CHN': 'China',
    'CIV': 'Ivory Coast',
    'CMR': 'Cameroon',
    'COD': 'Democratic Republic of the Congo',
    'COK': 'Cook Islands',
    'COL': 'Colombia',
    'COM': 'Comoros',
    'CPV': 'Cape Verde',
    'CRC': 'Costa Rica',
    'CRO': 'Croatia',
    'CRT': 'Crete',
    'CUB': 'Cuba',
    'CYP': 'Cyprus',
    'CZE': 'Czech Republic',
    'DEN': 'Denmark',
    'DJI': 'Djibouti',
    'DMA': 'Dominica',
    'DOM': 'Dominican Republic',
    'ECU': 'Ecuador',
    'EGY': 'Egypt',
    'ERI': 'Eritrea',
    'ESA': 'El Salvador',
    'ESP': 'Spain',
    'EST': 'Estonia',
    'ETH': 'Ethiopia',
    'FIJ': 'Fiji',
    'FIN': 'Finland',
    'FRA': 'France',
    'FSM': 'Micronesia',
    'GAB': 'Gabon',
    'GAM': 'Gambia',
    'GBR': 'Great Britain',
    'GBS': 'Guinea-Bissau',
    'GEO': 'Georgia',
    'GEQ': 'Equatorial Guinea',
    'GER': 'Germany',
    'GHA': 'Ghana',
    'GRE': 'Greece',
    'GRN': 'Grenada',
    'GUA': 'Guatemala',
    'GUI': 'Guinea',
    'GUM': 'Guam',
    'GUY': 'Guyana',
    'HAI': 'Haiti',
    'HKG': 'Hong Kong',
    'HON': 'Honduras',
    'HUN': 'Hungary',
    'INA': 'Indonesia',
    'IND': 'India',
    'IRI': 'Iran',
    'IRL': 'Ireland',
    'IRQ': 'Iraq',
    'ISL': 'Iceland',
    'ISR': 'Israel',
    'ISV': 'Virgin Islands',
    'ITA': 'Italy',
    'IVB': 'British Virgin Islands',
    'JAM': 'Jamaica',
    'JOR': 'Jordan',
    'JPN': 'Japan',
    'KAZ': 'Kazakhstan',
    'KEN': 'Kenya',
    'KGZ': 'Kyrgyzstan',
    'KIR': 'Kiribati',
    'KOR': 'South Korea',
    'KOS': 'Kosovo',
    'KSA': 'Saudi Arabia',
    'KUW': 'Kuwait',
    'LAO': 'Laos',
    'LAT': 'Latvia',
    'LBA': 'Libya',
    'LBR': 'Liberia',
    'LCA': 'Saint Lucia',
    'LES': 'Lesotho',
    'LIE': 'Liechtenstein',
    'LTU': 'Lithuania',
    'LUX': 'Luxembourg',
    'MAD': 'Madagascar',
    'MAL': 'Malaya',
    'MAR': 'Morocco',
    'MAS': 'Malaysia',
    'MAW': 'Malawi',
    'MDA': 'Moldova',
    'MDV': 'Maldives',
    'MEX': 'Mexico',
    'MGL': 'Mongolia',
    'MHL': 'Marshall Islands',
    'MKD': 'North Macedonia',
    'MLI': 'Mali',
    'MLT': 'Malta',
    'MNE': 'Montenegro',
    'MON': 'Monaco',
    'MOZ': 'Mozambique',
    'MRI': 'Mauritius',
    'MTN': 'Mauritania',
    'MYA': 'Myanmar',
    'NAM': 'Namibia',
    'NCA': 'Nicaragua',
    'NED': 'Netherlands',
    'NEP': 'Nepal',
    'NGR': 'Nigeria',
    'NIG': 'Niger',
    'NOR': 'Norway',
    'NRU': 'Nauru',
    'NZL': 'New Zealand',
    'NFL': 'Newfoundland',
    'OMA': 'Oman',
    'PAK': 'Pakistan',
    'PAN': 'Panama',
    'PAR': 'Paraguay',
    'PER': 'Peru',
    'PHI': 'Philippines',
    'PLE': 'Palestine',
    'PLW': 'Palau',
    'PNG': 'Papua New Guinea',
    'POL': 'Poland',
    'POR': 'Portugal',
    'PRK': 'North Korea',
    'PUR': 'Puerto Rico',
    'QAT': 'Qatar',
    'RHO': 'Rhodesia',
    'ROU': 'Romania',
    'RSA': 'South Africa',
    'RUS': 'Russia',
    'RWA': 'Rwanda',
    'SAA': 'Saar',
    'SAM': 'Samoa',
    'SEN': 'Senegal',
    'SEY': 'Seychelles',
    'SIN': 'Singapore',
    'SKN': 'Saint Kitts and Nevis',
    'SLE': 'Sierra Leone',
    'SLO': 'Slovenia',
    'SMR': 'San Marino',
    'SOL': 'Solomon Islands',
    'SRB': 'Serbia',
    'SRI': 'Sri Lanka',
    'STP': 'Sao Tome and Principe',
    'SUD': 'Sudan',
    'SUI': 'Switzerland',
    'SUR': 'Suriname',
    'SVK': 'Slovakia',
    'SWE': 'Sweden',
    'SWZ': 'Eswatini',
    'SYR': 'Syria',
    'TAN': 'Tanzania',
    'TGA': 'Tonga',
    'THA': 'Thailand',
    'TJK': 'Tajikistan',
    'TKM': 'Turkmenistan',
    'TLS': 'Timor-Leste',
    'TOG': 'Togo',
    'TPE': 'Chinese Taipei',
    'TTO': 'Trinidad and Tobago',
    'TUN': 'Tunisia',
    'TUR': 'Turkey',
    'TUV': 'Tuvalu',
    'UAE': 'United Arab Emirates',
    'UGA': 'Uganda',
    'UKR': 'Ukraine',
    'UNK': 'Unknown',
    'URU': 'Uruguay',
    'USA': 'United States',
    'UZB': 'Uzbekistan',
    'VAN': 'Vanuatu',
    'VEN': 'Venezuela',
    'VIE': 'Vietnam',
    'VIN': 'Saint Vincent and the Grenadines',
    'VNM': 'South Vietnam',
    'YEM': 'Yemen',
    'ZAM': 'Zambia',
    'ZIM': 'Zimbabwe'
}

# calculate the total number of participations for each country
athletes['NOC'] = athletes['NOC'].replace(noc_mapping)
athletes['NOC'] = athletes['NOC'].map(country_codes).fillna(athletes['NOC'])
athlete_counts = athletes.groupby(['Year', 'NOC']).size().reset_index(name='Athlete Count')
participation_counts = athletes.groupby(['Year', 'NOC']).size().reset_index(name='Athlete Count')
total_participations = participation_counts.groupby('NOC')['Year'].count().reset_index(name='Total Participations')

# find the countries that have never won a medal
medal_countries = athletes[athletes['Medal'].notna() & (athletes['Medal'] != 'No medal')]
countries_with_medals = medal_countries['NOC'].unique()
countries_without_medals = athletes[~athletes['NOC'].isin(countries_with_medals)]['NOC'].unique()
participation_counts = athletes.groupby(['Year', 'NOC']).size().reset_index(name='Athlete Count')
no_medal_participations = participation_counts[participation_counts['NOC'].isin(countries_without_medals)]

# calculate the total number of participations for each country
participation_counts = athletes.groupby(['Year', 'NOC']).size().reset_index(name='Athlete Count')
total_participations = participation_counts.groupby('NOC')['Year'].count().reset_index(name='Total Participations')

# merge the data
merged_data1 = pd.merge(no_medal_participations, total_participations, on='NOC', how='left')

